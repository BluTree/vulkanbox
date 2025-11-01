#include "input_system.hh"

#include "../log.hh"

#include <linux/input.h>
#include <wayland-client.h>

namespace vkb
{
	namespace
	{
		key convert_code_to_key(uint16_t scancode)
		{
			switch (scancode)
			{
				case KEY_ESC: return key::esc;
				case KEY_F1: return key::f1;
				case KEY_F2: return key::f2;
				case KEY_F3: return key::f3;
				case KEY_F4: return key::f4;
				case KEY_F5: return key::f5;
				case KEY_F6: return key::f6;
				case KEY_F7: return key::f7;
				case KEY_F8: return key::f8;
				case KEY_F9: return key::f9;
				case KEY_F10: return key::f10;
				case KEY_F11: return key::f11;
				case KEY_F12: return key::f12;
				case KEY_PRINT: return key::prnt_scrn;
				case KEY_SCROLLLOCK: return key::scrl_lck;
				case KEY_PAUSE: return key::pause_brk;
				case KEY_GRAVE: return key::backquote;
				case KEY_1: return key::one;
				case KEY_2: return key::two;
				case KEY_3: return key::three;
				case KEY_4: return key::four;
				case KEY_5: return key::five;
				case KEY_6: return key::six;
				case KEY_7: return key::seven;
				case KEY_8: return key::eight;
				case KEY_9: return key::nine;
				case KEY_0: return key::zero;
				case KEY_Q: return key::q;
				case KEY_W: return key::w;
				case KEY_E: return key::e;
				case KEY_R: return key::r;
				case KEY_T: return key::t;
				case KEY_Y: return key::y;
				case KEY_U: return key::u;
				case KEY_I: return key::i;
				case KEY_O: return key::o;
				case KEY_P: return key::p;
				case KEY_A: return key::a;
				case KEY_S: return key::s;
				case KEY_D: return key::d;
				case KEY_F: return key::f;
				case KEY_G: return key::g;
				case KEY_H: return key::h;
				case KEY_J: return key::j;
				case KEY_K: return key::k;
				case KEY_L: return key::l;
				case KEY_Z: return key::z;
				case KEY_X: return key::x;
				case KEY_C: return key::c;
				case KEY_V: return key::v;
				case KEY_B: return key::b;
				case KEY_N: return key::n;
				case KEY_M: return key::m;
				case KEY_MINUS: return key::dash;
				case KEY_EQUAL: return key::equal;
				case KEY_LEFTBRACE: return key::l_bracket;
				case KEY_RIGHTBRACE: return key::r_bracket;
				case KEY_SEMICOLON: return key::semicolon;
				case KEY_APOSTROPHE: return key::apostrophe;
				case KEY_BACKSLASH: return key::backslash;
				case KEY_COMMA: return key::comma;
				case KEY_DOT: return key::period;
				case KEY_SLASH: return key::slash;
				case KEY_TAB: return key::tab;
				case KEY_CAPSLOCK: return key::caps_lock;
				case KEY_LEFTSHIFT: return key::l_shift;
				case KEY_LEFTCTRL: return key::l_ctrl;
				// case KEY_0: return key::l_super;
				case KEY_LEFTALT: return key::l_alt;
				case KEY_SPACE: return key::space;
				case KEY_RIGHTALT: return key::r_alt;
				// case KEY_0: return key::r_super;
				case KEY_MENU: return key::menu;
				case KEY_RIGHTCTRL: return key::r_ctrl;
				case KEY_RIGHTSHIFT: return key::r_shift;
				case KEY_ENTER: return key::enter;
				case KEY_BACKSPACE: return key::backspace;
				case KEY_INSERT: return key::insert;
				case KEY_HOME: return key::home;
				case KEY_PAGEUP: return key::page_up;
				case KEY_DELETE: return key::del;
				case KEY_END: return key::end;
				case KEY_PAGEDOWN: return key::page_down;
				case KEY_UP: return key::up;
				case KEY_LEFT: return key::left;
				case KEY_DOWN: return key::down;
				case KEY_RIGHT: return key::right;
				case KEY_NUMLOCK: return key::num_lock;
				case KEY_KPSLASH: return key::num_div;
				case KEY_KPASTERISK: return key::num_mul;
				case KEY_KPMINUS: return key::num_min;
				case KEY_KPPLUS: return key::num_add;
				case KEY_KPENTER: return key::num_enter;
				case KEY_KPDOT: return key::num_period;
				case KEY_KP0: return key::num_0;
				case KEY_KP1: return key::num_1;
				case KEY_KP2: return key::num_2;
				case KEY_KP3: return key::num_3;
				case KEY_KP4: return key::num_4;
				case KEY_KP5: return key::num_5;
				case KEY_KP6: return key::num_6;
				case KEY_KP7: return key::num_7;
				case KEY_KP8: return key::num_8;
				case KEY_KP9: return key::num_9;
				default: return key::max_enum;
			}
		}
	} // namespace

	input_system::input_system() {}

	input_system::~input_system() {}

	void input_system::pointer_motion(wl_fixed_t x, wl_fixed_t y)
	{
		int32_t int_x = wl_fixed_to_int(x);
		int32_t int_y = wl_fixed_to_int(y);

		pos_rel_ = {pos_abs_.first - int_x, pos_abs_.second - int_y};
		pos_abs_ = {int_x, int_y};
	}

	void input_system::pointer_button(uint32_t button, uint32_t state)
	{
		key mouse_btn;
		switch (button)
		{
			case BTN_LEFT: mouse_btn = key::m1; break;
			case BTN_RIGHT: mouse_btn = key::m2; break;
			case BTN_MIDDLE: mouse_btn = key::m3; break;
			case BTN_FORWARD: mouse_btn = key::m4; break;
			case BTN_BACK: mouse_btn = key::m5; break;
			default: log::error("input: unknown mouse button: %x", button); return;
		}

		set_state(mouse_btn, state == WL_POINTER_BUTTON_STATE_PRESSED);
	}

	void input_system::pointer_axis([[maybe_unused]] uint32_t   axis,
	                                [[maybe_unused]] wl_fixed_t state)
	{}

	void input_system::keyboard_key(uint32_t k, uint32_t state)
	{
		key kb_key = convert_code_to_key(k);

		set_state(kb_key, state & WL_KEYBOARD_KEY_STATE_PRESSED);
	}
} // namespace vkb