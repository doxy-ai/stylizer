#pragma once

#include <stylizer/core/api.hpp>

namespace stylizer {

	template<typename T>
	struct storage;

	template<>
	struct storage<std::byte> : public std::vector<std::byte> {
		using super = std::vector<std::byte>;
		using super::super;

		size_t element_size = 1;

		size_t capacity() { return super::capacity() / element_size; }
		size_t size() { return super::size() / element_size; }

		std::byte& operator[](size_t index) {
			return super::operator[](index * element_size);
		}
		const std::byte& operator[](size_t index) const {
			return super::operator[](index * element_size);
		}

		std::span<std::byte> byte_span() { return {data(), super::size()}; }
	};

	template<typename T>
	struct storage : protected storage<std::byte> {
		using super = storage<std::byte>;

		storage() { element_size = sizeof(T); }
		storage(std::span<const T> span) : super((std::byte*)span.data(), ((std::byte*)span.data()) + span.size() * sizeof(T)) { element_size = sizeof(T); }
		storage(const storage&) = default;
		storage(storage&) = default;
		storage& operator=(const storage&) = default;
		storage& operator=(storage&) = default;
		using super::capacity;
		using super::size;
		using super::byte_span;

		template<typename... Targs>
		T& emplace_back(Targs... args) {
			auto inserted = super::insert(super::end(), sizeof(T), std::byte{0});
			return *(new(&*inserted) T(std::forward<Targs>(args)...));
		} 

		T& push_back(T& value) { return emplace_back(value); }
		T& push_back(T&& value) { return emplace_back(std::move(value)); }

		T& operator[](size_t index) { return (T&)super::operator[](index); }
		const T& operator[](size_t index) const { return (const T&)super::operator[](index); }

		T* data() { return (T*)super::data(); }
		const T* data() const { return (T*)super::data(); }
	};

}