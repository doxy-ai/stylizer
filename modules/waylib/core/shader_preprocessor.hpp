
// #define TCPP_IMPLEMENTATION
#include "thirdparty/tcppLibrary.hpp"
#include <webgpu/webgpu.hpp>

#include <set>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

#include "utility.hpp"

namespace wgsl_preprocess {

	namespace detail {
		inline std::string consolidate_whitespace(const std::string& in) {
			std::string out; out.reserve(in.size());
			for (size_t i = 0, count = 0, size = in.size(); i < size; ++i)
				if (std::isspace(in[i]))
					++count;
				else {
					if (count > 0) { // Replace multiple whitespace with a single space
						out += ' ';
						count = 0;
					}
					out += in[i];
				}

			// Handle trailing/leading whitespace characters
			while (out.length() >= 2 && out.back() == ' ') // NOTE: All whitespace has been converted to spaces at this point
				out.pop_back();
			if(out[0] == ' ') return out.substr(1);
			else return out;
		}

		inline std::string process_pragma_once(std::string data, const std::filesystem::path& path) {
			if(auto once = data.find("#pragma once"); once != std::string::npos) {
				std::string guard = path.string();
				std::transform(guard.begin(), guard.end(), guard.begin(), [](char c) -> char {
					if(!std::isalnum(c)) return '_';
					return std::toupper(c);
				});
				guard = "__" + guard + "_GUARD__";
				data.replace(once, 12, "#ifndef " + guard + "\n#define " + guard + "\n");
				data += "#endif //" + guard + "\n";
			}
			return data;
		}

		inline wl::result<std::string> read_entire_file(std::filesystem::path path, bool support_pragma_once) {
			std::ifstream fin(path);
			if(!fin) return wl::unexpected("Failed to open file `" + path.string() + "`... does it exist?");

			fin.seekg(0, std::ios::end);
			size_t size = fin.tellg();
			fin.seekg(0, std::ios::beg);

			std::string data(size + 1, '\n');
			fin.read(data.data(), size);
			if(support_pragma_once)
				return process_pragma_once(data, path);
			return data;
		}
	}

	struct shader_preprocessor {
		struct config {
			bool remove_comments = false;
			bool remove_whitespace = false;
			bool support_pragma_once = true;
			const char* path = nullptr;
		};

		std::unordered_map<std::filesystem::path, std::string> file_cache;

		std::set<std::filesystem::path> search_paths;
		std::set<std::string> defines = {};

		std::string defines_string() {
			std::string out = "\n";
			for(auto& def: defines)
				out += def + "\n";
			return out;
		}

		wl::result<std::string> process_from_memory(std::string_view data_, const config& config) {
			struct PreprocessFailed { std::string what; };

			auto data = std::make_unique<tcpp::StringInputStream>(defines_string() + std::string(data_) + "\n");
			tcpp::Lexer lexer(std::move(data));
			try {
				tcpp::Preprocessor preprocessor(lexer, {[](const tcpp::TErrorInfo& info){
					auto error = tcpp::ErrorTypeToString(info.mType);
					if (error.empty()) error = "Unknown error";
					error += " on line: " + std::to_string(info.mLine);
					// TODO: Include that line of the input!
					throw PreprocessFailed{error};
				}, [&](const std::string& path_, bool isSystem) -> tcpp::TInputStreamUniquePtr{
					std::filesystem::path path = path_;
					if(file_cache.find(path) != file_cache.end())
						return std::make_unique<tcpp::StringInputStream>(file_cache[path]);

					#define SHADER_PREPROCESSOR_NON_SYSTEM_PATHS {\
						if(config.path) {\
							auto relativeToConfig = std::filesystem::absolute(std::filesystem::path(config.path).parent_path() / path);\
							if(std::filesystem::exists(relativeToConfig)){\
								if(auto res = detail::read_entire_file(relativeToConfig, config.support_pragma_once); res)\
									return std::make_unique<tcpp::StringInputStream>(*res);\
								else throw PreprocessFailed{res.error()};\
							}\
						}\
						auto absolute = std::filesystem::absolute(path);\
						if(std::filesystem::exists(absolute)) {\
							if(auto res = detail::read_entire_file(absolute, config.support_pragma_once); res)\
								return std::make_unique<tcpp::StringInputStream>(*res);\
							else throw PreprocessFailed{res.error()};\
						}\
					}
					if(!isSystem) SHADER_PREPROCESSOR_NON_SYSTEM_PATHS

					for(auto system: search_paths) {
						system = system / path;
						if(std::filesystem::exists(system)) {
							if(auto res = detail::read_entire_file(system, config.support_pragma_once); res)
								return std::make_unique<tcpp::StringInputStream>(*res);
							else throw PreprocessFailed{res.error()};
						}
					}

					if(isSystem) SHADER_PREPROCESSOR_NON_SYSTEM_PATHS
					#undef SHADER_PREPROCESSOR_NON_SYSTEM_PATHS

					throw PreprocessFailed{"Included file `" + path.string() + "` could not be found!"};
				}, config.remove_comments});

				if(config.remove_whitespace) return detail::consolidate_whitespace(preprocessor.Process());
				return preprocessor.Process();
			} catch(PreprocessFailed fail) {
				return wl::unexpected(fail.what);
			}
		}

		wl::result<std::string> process_from_memory_and_cache(const std::string& data_, const std::filesystem::path& path, config config) {
			std::string data;
			if(config.support_pragma_once) data = detail::process_pragma_once(data_, path);
			else data = data_;

			auto str = path.string(); config.path = str.c_str();
			if(auto res = process_from_memory(data, config); res) {
				file_cache[path] = data;
				return *res;
			} else return wl::unexpected(res.error());
		}

		wl::result<std::string> process(const std::filesystem::path& path, const config& config) {
			if(file_cache.find(path) != file_cache.end())
				return file_cache[path];

			auto data = detail::read_entire_file(path, config.support_pragma_once);
			if(!data) return data;
			return process_from_memory_and_cache(*data, path, config);
		}

		shader_preprocessor& add_define(std::string_view name, std::string_view value) {
			defines.insert("#define " + std::string(name) + " " + std::string(value));
			return *this;
		}

		shader_preprocessor& initialize_platform_defines(WGPUAdapter adapter) {
			constexpr static auto quote = [](WGPUStringView view) { return "\"" + std::string(std::string_view(view.data, view.length)) + "\""; };

			wgpu::AdapterInfo info;
			static_cast<wgpu::Adapter>(adapter).getInfo(&info);
			add_define("WGPU_VENDOR", quote(info.vendor));
			add_define("WGPU_VENDOR", quote(info.vendor));
			add_define("WGPU_VENDOR_ID", std::to_string(info.vendorID));
			add_define("WGPU_ARCHITECTURE", quote(info.architecture));
			add_define("WGPU_DEVICE", quote(info.device));
			add_define("WGPU_DEVICE_ID", std::to_string(info.deviceID));
			add_define("WGPU_DESCRIPTION", quote(info.description));
			add_define("WGPU_ADAPTER_TYPE", std::to_string(info.adapterType));
			add_define("WGPU_BACKEND_TYPE", std::to_string(info.backendType));
			// add_define("WGPU_COMPATIBILITY_MODE", std::to_string(info.compatibilityMode));
			info.freeMembers();

			add_define("WGPU_ADAPTER_TYPE_DISCRETE_GPU", std::to_string(wgpu::AdapterType::DiscreteGPU));
			add_define("WGPU_ADAPTER_TYPE_INTEGRATED_GPU", std::to_string(wgpu::AdapterType::IntegratedGPU));
			add_define("WGPU_ADAPTER_TYPE_CPU", std::to_string(wgpu::AdapterType::CPU));

			add_define("WGPU_BACKEND_TYPE_WEBGPU", std::to_string(wgpu::BackendType::WebGPU));
			add_define("WGPU_BACKEND_TYPE_D3D11", std::to_string(wgpu::BackendType::D3D11));
			add_define("WGPU_BACKEND_TYPE_D3D12", std::to_string(wgpu::BackendType::D3D12));
			add_define("WGPU_BACKEND_TYPE_METAL", std::to_string(wgpu::BackendType::Metal));
			add_define("WGPU_BACKEND_TYPE_VULKAN", std::to_string(wgpu::BackendType::Vulkan));
			add_define("WGPU_BACKEND_TYPE_OPENGL", std::to_string(wgpu::BackendType::OpenGL));
			add_define("WGPU_BACKEND_TYPE_OPENGLES", std::to_string(wgpu::BackendType::OpenGLES));

			return *this;
		}

		// Note: This function is very expensive!
		shader_preprocessor& remove_define(std::string_view name) {
			auto iter = std::find_if(defines.begin(), defines.end(), [name](const std::string& define) {
				return std::search(define.begin(), define.end(), name.begin(), name.end()) != define.end();
			});

			if(iter != defines.end())
				defines.erase(iter);
			return *this;
		}
	};
}