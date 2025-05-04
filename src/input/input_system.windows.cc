#include "input_system.hh"

#include "../log.hh"
#include "keys.hh"

#include <array.hh>

#include <string.h>
#include <win32/misc.h>
#include <win32/rawinput.h>

namespace vkb
{
	namespace
	{
		key convert_scancode_to_key(uint16_t scancode)
		{
			switch (scancode)
			{
				case 0x0001: return key::esc;
				case 0x003B: return key::f1;
				case 0x003C: return key::f2;
				case 0x003D: return key::f3;
				case 0x003E: return key::f4;
				case 0x003F: return key::f5;
				case 0x0040: return key::f6;
				case 0x0041: return key::f7;
				case 0x0042: return key::f8;
				case 0x0043: return key::f9;
				case 0x0044: return key::f10;
				case 0x0057: return key::f11;
				case 0x0058: return key::f12;
				case 0xE037: return key::prnt_scrn;
				case 0x0046: return key::scrl_lck;
				case 0x0045: return key::pause_brk;
				case 0x0029: return key::backquote;
				case 0x0002: return key::one;
				case 0x0003: return key::two;
				case 0x0004: return key::three;
				case 0x0005: return key::four;
				case 0x0006: return key::five;
				case 0x0007: return key::six;
				case 0x0008: return key::seven;
				case 0x0009: return key::eight;
				case 0x000A: return key::nine;
				case 0x000B: return key::zero;
				case 0x0010: return key::q;
				case 0x0011: return key::w;
				case 0x0012: return key::e;
				case 0x0013: return key::r;
				case 0x0014: return key::t;
				case 0x0015: return key::y;
				case 0x0016: return key::u;
				case 0x0017: return key::i;
				case 0x0018: return key::o;
				case 0x0019: return key::p;
				case 0x001E: return key::a;
				case 0x001F: return key::s;
				case 0x0020: return key::d;
				case 0x0021: return key::f;
				case 0x0022: return key::g;
				case 0x0023: return key::h;
				case 0x0024: return key::j;
				case 0x0025: return key::k;
				case 0x0026: return key::l;
				case 0x002C: return key::z;
				case 0x002D: return key::x;
				case 0x002E: return key::c;
				case 0x002F: return key::v;
				case 0x0030: return key::b;
				case 0x0031: return key::n;
				case 0x0032: return key::m;
				case 0x000C: return key::dash;
				case 0x000D: return key::equal;
				case 0x001A: return key::l_bracket;
				case 0x001B: return key::r_bracket;
				case 0x0027: return key::semicolon;
				case 0x0028: return key::apostrophe;
				case 0x002B: return key::backslash;
				case 0x0033: return key::comma;
				case 0x0034: return key::period;
				case 0x0035: return key::slash;
				case 0x000F: return key::tab;
				case 0x003A: return key::caps_lock;
				case 0x002A: return key::l_shift;
				case 0x001D: return key::l_ctrl;
				case 0xE05B: return key::l_super;
				case 0x0038: return key::l_alt;
				case 0x0039: return key::space;
				case 0xE038: return key::r_alt;
				case 0xE05C: return key::r_super;
				case 0xE05D: return key::menu;
				case 0xE01D: return key::r_ctrl;
				case 0x0036: return key::r_shift;
				case 0x001C: return key::enter;
				case 0x000E: return key::backspace;
				case 0xE052: return key::insert;
				case 0xE047: return key::home;
				case 0xE049: return key::page_up;
				case 0xE053: return key::del;
				case 0xE04F: return key::end;
				case 0xE051: return key::page_down;
				case 0xE048: return key::up;
				case 0xE04B: return key::left;
				case 0xE050: return key::down;
				case 0xE04D: return key::right;
				case 0xE045: return key::num_lock;
				case 0xE035: return key::num_div;
				case 0x0037: return key::num_mul;
				case 0x004A: return key::num_min;
				case 0x004E: return key::num_add;
				case 0xE01C: return key::num_enter;
				case 0x0053: return key::num_period;
				case 0x0052: return key::num_0;
				case 0x004F: return key::num_1;
				case 0x0050: return key::num_2;
				case 0x0051: return key::num_3;
				case 0x004B: return key::num_4;
				case 0x004C: return key::num_5;
				case 0x004D: return key::num_6;
				case 0x0047: return key::num_7;
				case 0x0048: return key::num_8;
				case 0x0049: return key::num_9;
				default: return key::max_enum;
			}
		}
	} // namespace

	input_system::input_system()
	{
		RAWINPUTDEVICE inputs[2];

		inputs[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		inputs[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		inputs[0].dwFlags = 0 /*RIDEV_NOLEGACY*/;
		inputs[0].hwndTarget = 0;

		inputs[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		inputs[1].usUsage = HID_USAGE_GENERIC_MOUSE;
		inputs[1].dwFlags = 0 /*RIDEV_NOLEGACY*/;
		inputs[1].hwndTarget = 0;

		log::assert(RegisterRawInputDevices(inputs, 2, sizeof(RAWINPUTDEVICE)) != 0,
		            "Cannot initialize Raw Input: Error %lu", GetLastError());

		memset(key_states_, 0, (mc::to_underlying(key::max_enum) * 2) / 8);
	}

	input_system::~input_system() {}

	void input_system::clear_transitions()
	{
		for (mc::underlying_type<key> i {0}; i < mc::to_underlying(key::max_enum); ++i)
			key_states_[i * 2 / 8] &= ~(1 << (i * 2 % 8 + 1));

		wheel_ = {0.f, 0.f};
		pos_rel_ = {0, 0};
	}

	bool input_system::pressed(key k) const
	{
		return (key_states_[mc::to_underlying(k) * 2 / 8] &
		        (1 << (mc::to_underlying(k) * 2 % 8))) != 0;
	}

	bool input_system::released(key k) const
	{
		return (key_states_[mc::to_underlying(k) * 2 / 8] &
		        (1 << (mc::to_underlying(k) * 2 % 8))) == 0;
	}

	bool input_system::just_pressed(key k) const
	{
		return ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << (mc::to_underlying(k) * 2 % 8))) != 0) &&
		       ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << ((mc::to_underlying(k) * 2 + 1) % 8))) != 0);
	}

	bool input_system::just_released(key k) const
	{
		return ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << (mc::to_underlying(k) * 2 % 8))) == 0) &&
		       ((key_states_[mc::to_underlying(k) * 2 / 8] &
		         (1 << ((mc::to_underlying(k) * 2 + 1) % 8))) != 0);
	}

	mc::pair<float, float> input_system::mouse_wheel() const
	{
		return wheel_;
	}

	mc::pair<int32_t, int32_t> input_system::mouse_pos() const
	{
		return pos_abs_;
	}

	mc::pair<int32_t, int32_t> input_system::mouse_delta() const
	{
		return pos_rel_;
	}

	void input_system::handle_event([[maybe_unused]] uint64_t w_param, uint64_t l_param)
	{
		uint32_t size {0};

		GetRawInputData(reinterpret_cast<HRAWINPUT>(l_param), RID_INPUT, nullptr, &size,
		                sizeof(RAWINPUTHEADER));
		uint8_t* bytes = new uint8_t[size];
		GetRawInputData(reinterpret_cast<HRAWINPUT>(l_param), RID_INPUT, bytes, &size,
		                sizeof(RAWINPUTHEADER));

		RAWINPUT* data = reinterpret_cast<RAWINPUT*>(bytes);

		if (data->header.dwType == RIM_TYPEKEYBOARD &&
		    data->data.keyboard.MakeCode != KEYBOARD_OVERRUN_MAKE_CODE)
		{
			uint16_t sc = data->data.keyboard.MakeCode;
			if ((data->data.keyboard.Flags & RI_KEY_E0) != 0)
				sc |= 0xe000;
			else if ((data->data.keyboard.Flags & RI_KEY_E1) != 0)
				sc |= 0xe100;

			key  k = convert_scancode_to_key(sc);
			bool last_pressed = (key_states_[mc::to_underlying(k) / 8] &
			                     (1 << (mc::to_underlying(k) % 8))) != 0;

			if ((data->data.keyboard.Flags & RI_KEY_BREAK) != 0)
			{
				key_states_[mc::to_underlying(k) * 2 / 8] &=
					~(1 << (mc::to_underlying(k) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(k) * 2 / 8] |=
						(1 << (mc::to_underlying(k) * 2 % 8 + 1));
			}
			else
			{
				key_states_[mc::to_underlying(k) * 2 / 8] |=
					(1 << (mc::to_underlying(k) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(k) * 2 / 8] |=
						(1 << (mc::to_underlying(k) * 2 % 8 + 1));
			}
		}
		else if (data->header.dwType == RIM_TYPEMOUSE)
		{
			if (data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
			{
				// TODO Handle virtual desktop
				pos_abs_.first = data->data.mouse.lLastX;
				pos_abs_.second = data->data.mouse.lLastY;
			}
			else /* MOUSE_MOVE_RELATIVE */
			{
				pos_rel_.first += data->data.mouse.lLastX;
				pos_rel_.second += data->data.mouse.lLastY;
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) != 0)
			{
				wheel_.first += static_cast<int16_t>(data->data.mouse.usButtonData) /
				                static_cast<float>(WHEEL_DELTA);
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_HWHEEL) != 0)
			{
				wheel_.second += static_cast<int16_t>(data->data.mouse.usButtonData) /
				                 static_cast<float>(WHEEL_DELTA);
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m1) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m1) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m1) * 2 / 8] |=
					(1 << (mc::to_underlying(key::m1) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(key::m1) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m1) * 2 % 8 + 1));
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m1) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m1) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m1) * 2 / 8] &=
					~(1 << (mc::to_underlying(key::m1) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(key::m1) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m1) * 2 % 8 + 1));
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m2) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m2) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m2) * 2 / 8] |=
					(1 << (mc::to_underlying(key::m2) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(key::m2) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m2) * 2 % 8 + 1));
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m2) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m2) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m2) * 2 / 8] &=
					~(1 << (mc::to_underlying(key::m2) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(key::m2) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m2) * 2 % 8 + 1));
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m3) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m3) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m3) * 2 / 8] |=
					(1 << (mc::to_underlying(key::m3) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(key::m3) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m3) * 2 % 8 + 1));
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m3) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m3) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m3) * 2 / 8] &=
					~(1 << (mc::to_underlying(key::m3) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(key::m3) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m3) * 2 % 8 + 1));
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m4) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m4) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m4) * 2 / 8] |=
					(1 << (mc::to_underlying(key::m4) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(key::m4) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m4) * 2 % 8 + 1));
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m4) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m4) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m4) * 2 / 8] &=
					~(1 << (mc::to_underlying(key::m4) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(key::m4) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m4) * 2 % 8 + 1));
			}

			if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m5) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m5) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m5) * 2 / 8] |=
					(1 << (mc::to_underlying(key::m5) * 2 % 8));

				if (!last_pressed)
					key_states_[mc::to_underlying(key::m5) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m5) * 2 % 8 + 1));
			}
			else if ((data->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) != 0)
			{
				bool last_pressed = (key_states_[mc::to_underlying(key::m5) * 2 / 8] &
				                     (1 << (mc::to_underlying(key::m5) * 2 % 8))) != 0;

				key_states_[mc::to_underlying(key::m5) * 2 / 8] &=
					~(1 << (mc::to_underlying(key::m5) * 2 % 8));

				if (last_pressed)
					key_states_[mc::to_underlying(key::m5) * 2 / 8] |=
						(1 << (mc::to_underlying(key::m5) * 2 % 8 + 1));
			}
		}

		delete[] bytes;
	}
} // namespace vkb