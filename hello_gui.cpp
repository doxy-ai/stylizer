import std.compat;
import stylizer;

int main() {
    auto ctx = stylizer::context::create_default_with_error_handler();

    ctx.send_info("Hello World");

    stylizer::time time;
    time.update();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    time.update();
}