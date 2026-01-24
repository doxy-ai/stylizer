import std.compat;
import stylizer;

int main() {
    auto ctx = stylizer::context::create_default_with_error_handler();

    ctx.send_info("Hello World");
}