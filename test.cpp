#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state().throw_if_error();
	window.reconfigure_surface_on_resize(state);

	WAYLIB_MAIN_LOOP(!window.should_close(),
		wl::thread_pool::enqueue([]{
			std::cout << "Hello 世界" << std::endl;
		});
	);
}