void f(double w) {
	for (unsigned int i = 0; i < size; ++i) {
		controller.push_address_to_cache(get_imag_address(xx, i)); // get value from xx[i]
		controller.push_address_to_cache(get_imag_address(x, i)); // get value from x[i]
		x[i] = xx[i] * w + x[i];
		controller.push_address_to_cache(get_imag_address(x, i)); // write value to x[i]
	}
	for (unsigned int i = 0; i < size; ++i) {
		controller.push_address_to_cache(get_imag_address(yy, i)); // get value from yy[i]
		controller.push_address_to_cache(get_imag_address(y, i)); // get value from y[i]
		y[i] = yy[i] * w + y[i];
		controller.push_address_to_cache(get_imag_address(y, i)); // write value to y[i]
	}
	for (unsigned int i = 0; i < size; ++i) {
		controller.push_address_to_cache(get_imag_address(zz, i)); // get value from zz[i]
		controller.push_address_to_cache(get_imag_address(z, i)); // get value from z[i]
		z[i] = zz[i] * w + z[i];
		controller.push_address_to_cache(get_imag_address(z, i)); // write value to z[i]
	}
}