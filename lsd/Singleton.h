#pragma once

template <typename t>
class singleton {
public:
	static t& get() {
		static t instance{};
		return instance;
	}
};