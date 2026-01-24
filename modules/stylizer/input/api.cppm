module;

#include <stylizer/core/util/reaction.hpp>

export module stylizer.input;

import std.compat;
import stdmath;
import stdmath.slang;
import stylizer;

namespace stylizer {

	export struct input {

		using device_t = size_t;
		static constexpr device_t all_devices = ~0;

		virtual void update_time(const time& time) {
			reaction::batchExecute([&] {
				update_if_different(get_floating(current_time), time.total);
				update_if_different(get_integer(current_frame), int64_t{time.frame});
			});
		}

		virtual void register_event_listener(context& ctx) = 0;

		enum boolean {
			// From: SDL_keycode.h
			key_unknown                = 0x00000000u, /**< 0 */
			key_return                 = 0x0000000du, /**< '\r' */
			key_escape                 = 0x0000001bu, /**< '\x1B' */
			key_backspace              = 0x00000008u, /**< '\b' */
			key_tab                    = 0x00000009u, /**< '\t' */
			key_space                  = 0x00000020u, /**< ' ' */
			key_exclaim                = 0x00000021u, /**< '!' */
			key_dblapostrophe          = 0x00000022u, /**< '"' */
			key_hash                   = 0x00000023u, /**< '#' */
			key_dollar                 = 0x00000024u, /**< '$' */
			key_percent                = 0x00000025u, /**< '%' */
			key_ampersand              = 0x00000026u, /**< '&' */
			key_apostrophe             = 0x00000027u, /**< '\'' */
			key_leftparen              = 0x00000028u, /**< '(' */
			key_rightparen             = 0x00000029u, /**< ')' */
			key_asterisk               = 0x0000002au, /**< '*' */
			key_plus                   = 0x0000002bu, /**< '+' */
			key_comma                  = 0x0000002cu, /**< ',' */
			key_minus                  = 0x0000002du, /**< '-' */
			key_period                 = 0x0000002eu, /**< '.' */
			key_slash                  = 0x0000002fu, /**< '/' */
			key_0                      = 0x00000030u, /**< '0' */
			key_1                      = 0x00000031u, /**< '1' */
			key_2                      = 0x00000032u, /**< '2' */
			key_3                      = 0x00000033u, /**< '3' */
			key_4                      = 0x00000034u, /**< '4' */
			key_5                      = 0x00000035u, /**< '5' */
			key_6                      = 0x00000036u, /**< '6' */
			key_7                      = 0x00000037u, /**< '7' */
			key_8                      = 0x00000038u, /**< '8' */
			key_9                      = 0x00000039u, /**< '9' */
			key_colon                  = 0x0000003au, /**< ':' */
			key_semicolon              = 0x0000003bu, /**< ';' */
			key_less                   = 0x0000003cu, /**< '<' */
			key_equals                 = 0x0000003du, /**< '=' */
			key_greater                = 0x0000003eu, /**< '>' */
			key_question               = 0x0000003fu, /**< '?' */
			key_at                     = 0x00000040u, /**< '@' */
			key_leftbracket            = 0x0000005bu, /**< '[' */
			key_backslash              = 0x0000005cu, /**< '\\' */
			key_rightbracket           = 0x0000005du, /**< ']' */
			key_caret                  = 0x0000005eu, /**< '^' */
			key_underscore             = 0x0000005fu, /**< '_' */
			key_grave                  = 0x00000060u, /**< '`' */
			key_a                      = 0x00000061u, /**< 'a' */
			key_b                      = 0x00000062u, /**< 'b' */
			key_c                      = 0x00000063u, /**< 'c' */
			key_d                      = 0x00000064u, /**< 'd' */
			key_e                      = 0x00000065u, /**< 'e' */
			key_f                      = 0x00000066u, /**< 'f' */
			key_g                      = 0x00000067u, /**< 'g' */
			key_h                      = 0x00000068u, /**< 'h' */
			key_i                      = 0x00000069u, /**< 'i' */
			key_j                      = 0x0000006au, /**< 'j' */
			key_k                      = 0x0000006bu, /**< 'k' */
			key_l                      = 0x0000006cu, /**< 'l' */
			key_m                      = 0x0000006du, /**< 'm' */
			key_n                      = 0x0000006eu, /**< 'n' */
			key_o                      = 0x0000006fu, /**< 'o' */
			key_p                      = 0x00000070u, /**< 'p' */
			key_q                      = 0x00000071u, /**< 'q' */
			key_r                      = 0x00000072u, /**< 'r' */
			key_s                      = 0x00000073u, /**< 's' */
			key_t                      = 0x00000074u, /**< 't' */
			key_u                      = 0x00000075u, /**< 'u' */
			key_v                      = 0x00000076u, /**< 'v' */
			key_w                      = 0x00000077u, /**< 'w' */
			key_x                      = 0x00000078u, /**< 'x' */
			key_y                      = 0x00000079u, /**< 'y' */
			key_z                      = 0x0000007au, /**< 'z' */
			key_leftbrace              = 0x0000007bu, /**< '{' */
			key_pipe                   = 0x0000007cu, /**< '|' */
			key_rightbrace             = 0x0000007du, /**< '}' */
			key_tilde                  = 0x0000007eu, /**< '~' */
			key_delete                 = 0x0000007fu, /**< '\x7F' */
			key_plusminus              = 0x000000b1u, /**< '\xB1' */
			key_capslock               = 0x40000039u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CAPSLOCK) */
			key_f1                     = 0x4000003au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1) */
			key_f2                     = 0x4000003bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F2) */
			key_f3                     = 0x4000003cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F3) */
			key_f4                     = 0x4000003du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F4) */
			key_f5                     = 0x4000003eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F5) */
			key_f6                     = 0x4000003fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F6) */
			key_f7                     = 0x40000040u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F7) */
			key_f8                     = 0x40000041u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F8) */
			key_f9                     = 0x40000042u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F9) */
			key_f10                    = 0x40000043u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F10) */
			key_f11                    = 0x40000044u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F11) */
			key_f12                    = 0x40000045u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F12) */
			key_printscreen            = 0x40000046u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRINTSCREEN) */
			key_scrolllock             = 0x40000047u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SCROLLLOCK) */
			key_pause                  = 0x40000048u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAUSE) */
			key_insert                 = 0x40000049u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_INSERT) */
			key_home                   = 0x4000004au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HOME) */
			key_pageup                 = 0x4000004bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEUP) */
			key_end                    = 0x4000004du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_END) */
			key_pagedown               = 0x4000004eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PAGEDOWN) */
			key_right                  = 0x4000004fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RIGHT) */
			key_left                   = 0x40000050u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LEFT) */
			key_down                   = 0x40000051u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DOWN) */
			key_up                     = 0x40000052u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UP) */
			key_numlockclear           = 0x40000053u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_NUMLOCKCLEAR) */
			key_kp_divide              = 0x40000054u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DIVIDE) */
			key_kp_multiply            = 0x40000055u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MULTIPLY) */
			key_kp_minus               = 0x40000056u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MINUS) */
			key_kp_plus                = 0x40000057u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUS) */
			key_kp_enter               = 0x40000058u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_ENTER) */
			key_kp_1                   = 0x40000059u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_1) */
			key_kp_2                   = 0x4000005au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_2) */
			key_kp_3                   = 0x4000005bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_3) */
			key_kp_4                   = 0x4000005cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_4) */
			key_kp_5                   = 0x4000005du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_5) */
			key_kp_6                   = 0x4000005eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_6) */
			key_kp_7                   = 0x4000005fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_7) */
			key_kp_8                   = 0x40000060u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_8) */
			key_kp_9                   = 0x40000061u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_9) */
			key_kp_0                   = 0x40000062u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_0) */
			key_kp_period              = 0x40000063u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERIOD) */
			key_application            = 0x40000065u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_APPLICATION) */
			key_power                  = 0x40000066u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_POWER) */
			key_kp_equals              = 0x40000067u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALS) */
			key_f13                    = 0x40000068u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F13) */
			key_f14                    = 0x40000069u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F14) */
			key_f15                    = 0x4000006au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F15) */
			key_f16                    = 0x4000006bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F16) */
			key_f17                    = 0x4000006cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F17) */
			key_f18                    = 0x4000006du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F18) */
			key_f19                    = 0x4000006eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F19) */
			key_f20                    = 0x4000006fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F20) */
			key_f21                    = 0x40000070u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F21) */
			key_f22                    = 0x40000071u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F22) */
			key_f23                    = 0x40000072u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F23) */
			key_f24                    = 0x40000073u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F24) */
			key_execute                = 0x40000074u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXECUTE) */
			key_help                   = 0x40000075u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_HELP) */
			key_menu                   = 0x40000076u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MENU) */
			key_select                 = 0x40000077u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SELECT) */
			key_stop                   = 0x40000078u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_STOP) */
			key_again                  = 0x40000079u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AGAIN) */
			key_undo                   = 0x4000007au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_UNDO) */
			key_cut                    = 0x4000007bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CUT) */
			key_copy                   = 0x4000007cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_COPY) */
			key_paste                  = 0x4000007du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PASTE) */
			key_find                   = 0x4000007eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_FIND) */
			key_mute                   = 0x4000007fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MUTE) */
			key_volumeup               = 0x40000080u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEUP) */
			key_volumedown             = 0x40000081u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_VOLUMEDOWN) */
			key_kp_comma               = 0x40000085u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COMMA) */
			key_kp_equalsas400         = 0x40000086u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EQUALSAS400) */
			key_alterase               = 0x40000099u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ALTERASE) */
			key_sysreq                 = 0x4000009au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SYSREQ) */
			key_cancel                 = 0x4000009bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CANCEL) */
			key_clear                  = 0x4000009cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEAR) */
			key_prior                  = 0x4000009du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_PRIOR) */
			key_return2                = 0x4000009eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RETURN2) */
			key_separator              = 0x4000009fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SEPARATOR) */
			key_out                    = 0x400000a0u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OUT) */
			key_oper                   = 0x400000a1u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_OPER) */
			key_clearagain             = 0x400000a2u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CLEARAGAIN) */
			key_crsel                  = 0x400000a3u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CRSEL) */
			key_exsel                  = 0x400000a4u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_EXSEL) */
			key_kp_00                  = 0x400000b0u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_00) */
			key_kp_000                 = 0x400000b1u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_000) */
			key_thousandsseparator     = 0x400000b2u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_THOUSANDSSEPARATOR) */
			key_decimalseparator       = 0x400000b3u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_DECIMALSEPARATOR) */
			key_currencyunit           = 0x400000b4u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYUNIT) */
			key_currencysubunit        = 0x400000b5u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CURRENCYSUBUNIT) */
			key_kp_leftparen           = 0x400000b6u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTPAREN) */
			key_kp_rightparen          = 0x400000b7u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTPAREN) */
			key_kp_leftbrace           = 0x400000b8u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LEFTBRACE) */
			key_kp_rightbrace          = 0x400000b9u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_RIGHTBRACE) */
			key_kp_tab                 = 0x400000bau, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_TAB) */
			key_kp_backspace           = 0x400000bbu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BACKSPACE) */
			key_kp_a                   = 0x400000bcu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_A) */
			key_kp_b                   = 0x400000bdu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_B) */
			key_kp_c                   = 0x400000beu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_C) */
			key_kp_d                   = 0x400000bfu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_D) */
			key_kp_e                   = 0x400000c0u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_E) */
			key_kp_f                   = 0x400000c1u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_F) */
			key_kp_xor                 = 0x400000c2u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_XOR) */
			key_kp_power               = 0x400000c3u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_POWER) */
			key_kp_percent             = 0x400000c4u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PERCENT) */
			key_kp_less                = 0x400000c5u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_LESS) */
			key_kp_greater             = 0x400000c6u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_GREATER) */
			key_kp_ampersand           = 0x400000c7u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AMPERSAND) */
			key_kp_dblampersand        = 0x400000c8u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLAMPERSAND) */
			key_kp_verticalbar         = 0x400000c9u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_VERTICALBAR) */
			key_kp_dblverticalbar      = 0x400000cau, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DBLVERTICALBAR) */
			key_kp_colon               = 0x400000cbu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_COLON) */
			key_kp_hash                = 0x400000ccu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HASH) */
			key_kp_space               = 0x400000cdu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_SPACE) */
			key_kp_at                  = 0x400000ceu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_AT) */
			key_kp_exclam              = 0x400000cfu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_EXCLAM) */
			key_kp_memstore            = 0x400000d0u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSTORE) */
			key_kp_memrecall           = 0x400000d1u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMRECALL) */
			key_kp_memclear            = 0x400000d2u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMCLEAR) */
			key_kp_memadd              = 0x400000d3u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMADD) */
			key_kp_memsubtract         = 0x400000d4u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMSUBTRACT) */
			key_kp_memmultiply         = 0x400000d5u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMMULTIPLY) */
			key_kp_memdivide           = 0x400000d6u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_MEMDIVIDE) */
			key_kp_plusminus           = 0x400000d7u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_PLUSMINUS) */
			key_kp_clear               = 0x400000d8u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEAR) */
			key_kp_clearentry          = 0x400000d9u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_CLEARENTRY) */
			key_kp_binary              = 0x400000dau, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_BINARY) */
			key_kp_octal               = 0x400000dbu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_OCTAL) */
			key_kp_decimal             = 0x400000dcu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_DECIMAL) */
			key_kp_hexadecimal         = 0x400000ddu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_KP_HEXADECIMAL) */
			key_lctrl                  = 0x400000e0u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LCTRL) */
			key_lshift                 = 0x400000e1u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LSHIFT) */
			key_lalt                   = 0x400000e2u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LALT) */
			key_lgui                   = 0x400000e3u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_LGUI) */
			key_rctrl                  = 0x400000e4u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RCTRL) */
			key_rshift                 = 0x400000e5u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RSHIFT) */
			key_ralt                   = 0x400000e6u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RALT) */
			key_rgui                   = 0x400000e7u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_RGUI) */
			key_mode                   = 0x40000101u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MODE) */
			key_sleep                  = 0x40000102u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SLEEP) */
			key_wake                   = 0x40000103u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_WAKE) */
			key_channel_increment      = 0x40000104u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_INCREMENT) */
			key_channel_decrement      = 0x40000105u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CHANNEL_DECREMENT) */
			key_media_play             = 0x40000106u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY) */
			key_media_pause            = 0x40000107u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PAUSE) */
			key_media_record           = 0x40000108u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_RECORD) */
			key_media_fast_forward     = 0x40000109u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_FAST_FORWARD) */
			key_media_rewind           = 0x4000010au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_REWIND) */
			key_media_next_track       = 0x4000010bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_NEXT_TRACK) */
			key_media_previous_track   = 0x4000010cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK) */
			key_media_stop             = 0x4000010du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_STOP) */
			key_media_eject            = 0x4000010eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_EJECT) */
			key_media_play_pause       = 0x4000010fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_PLAY_PAUSE) */
			key_media_select           = 0x40000110u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_MEDIA_SELECT) */
			key_ac_new                 = 0x40000111u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_NEW) */
			key_ac_open                = 0x40000112u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_OPEN) */
			key_ac_close               = 0x40000113u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_CLOSE) */
			key_ac_exit                = 0x40000114u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_EXIT) */
			key_ac_save                = 0x40000115u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SAVE) */
			key_ac_print               = 0x40000116u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PRINT) */
			key_ac_properties          = 0x40000117u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_PROPERTIES) */
			key_ac_search              = 0x40000118u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_SEARCH) */
			key_ac_home                = 0x40000119u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_HOME) */
			key_ac_back                = 0x4000011au, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BACK) */
			key_ac_forward             = 0x4000011bu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_FORWARD) */
			key_ac_stop                = 0x4000011cu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_STOP) */
			key_ac_refresh             = 0x4000011du, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_REFRESH) */
			key_ac_bookmarks           = 0x4000011eu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_AC_BOOKMARKS) */
			key_softleft               = 0x4000011fu, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTLEFT) */
			key_softright              = 0x40000120u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_SOFTRIGHT) */
			key_call                   = 0x40000121u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_CALL) */
			key_endcall                = 0x40000122u, /**< SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ENDCALL) */
			key_left_tab               = 0x20000001u, /**< Extended key Left Tab */
			key_level5_shift           = 0x20000002u, /**< Extended key Level 5 Shift */
			key_multi_key_compose      = 0x20000003u, /**< Extended key Multi-key Compose */
			key_lmeta                  = 0x20000004u, /**< Extended key Left Meta */
			key_rmeta                  = 0x20000005u, /**< Extended key Right Meta */
			key_lhyper                 = 0x20000006u, /**< Extended key Left Hyper */
			key_rhyper                 = 0x20000007u, /**< Extended key Right Hyper */
		};

		virtual reaction::Var<bool>& get_boolean(boolean what, device_t devices = all_devices) = 0;

		enum floating {
			current_time
		};

		virtual reaction::Var<float>& get_floating(floating what, device_t devices = all_devices) = 0;

		enum integer {
			current_frame,
			current_device
		};

		virtual reaction::Var<int64_t>& get_integer(integer what, device_t devices = all_devices) = 0;

		enum vector {
			mouse_position,
			mouse_wheel,
		};

		virtual reaction::Var<stdmath::float2>& get_vector(vector what, device_t devices = all_devices) = 0;

		// Tracks the change from the last event
		template<reaction::IsReact Taction>
		static auto delta(Taction action) {
			return reaction::calc([](auto current) {
				static auto last = current;
				auto delta = current - last;
				last = current;
				return delta;
			}, action);
		}

		// Tracks the change that occurs per frame (requires time to be updated using update_time)
		template<reaction::IsReact Taction, size_t frames_to_keep = 5>
		auto per_frame_delta(Taction action) {
			return reaction::calc([](auto current, size_t frame) {
				static std::array<decltype(current), frames_to_keep> frames;
				static size_t back_frame = 0;

				size_t from_back = std::min(back_frame - frame, frames.size() - 1);
				size_t index = frames.size() - from_back - 1;

				auto delta = current - frames[index];
				if(frame > back_frame) {
					std::memmove(frames.data(), frames.data() + 1, sizeof(frames[0]) * frames.size() - 1);
					frames.back() = current;
					back_frame = frame;
				}
				return delta;
			}, action, get_integer(current_frame));
		}

		// 0 to indicate no keys, 1 to indicate any keys
		template<reaction::IsReact... Tactions>
		static auto any_of(Tactions... actions) {
			return reaction::expr((actions || ...));
		}

		template<reaction::IsReact... Tactions>
		static auto all_of(Tactions... actions) {
			return reaction::expr((actions && ...));
		}

		// The number of keys in the combo that are pressed
		template<reaction::IsReact... Tactions>
		static auto combination(Tactions... actions) {
			return reaction::expr((actions + ...));
		}

		// Computes pos - neg, or in other words, converts inputs to [-1, 1]
		template<reaction::IsReact Tpositive, reaction::IsReact Tnegative>
		static auto buttons_to_axis(Tpositive pos, Tnegative neg) {
			return reaction::calc([](auto pos, auto neg) {
				return (int)pos - (int)neg;
			}, pos, neg);
		}

		template<reaction::IsReact Taction>
		requires(!requires (Taction v) { {v()} -> std::same_as<stdmath::float2>; })
		static auto normalize(Taction action, float epsilon = .001) {
			return reaction::calc([epsilon](auto v) {
				return (float)stdmath::normalize(v, epsilon);
			}, action);
		}

		template<reaction::IsReact Tx, reaction::IsReact Ty>
		static auto floats_to_vector(Tx x, Ty y) {
			return reaction::calc([](auto x, auto y) {
				return stdmath::float2{(float)x, (float)y};
			}, x, y);
		}

		template<reaction::IsReact Tup, reaction::IsReact Tdown, reaction::IsReact Tleft, reaction::IsReact Tright>
		static auto buttons_to_vector(Tup up, Tdown down, Tleft left, Tright right) {
			return floats_to_vector(buttons_to_axis(right, left), buttons_to_axis(up, down));
		}

		template<reaction::IsReact Tvector>
		requires(requires (Tvector v) { {v()} -> std::same_as<stdmath::float2>; })
		static auto x_axis(Tvector v) {
			return reaction::calc([](const auto& v) {
				return v.x;
			}, v);
		}

		template<reaction::IsReact Tvector>
		requires(requires (Tvector v) { {v()} -> std::same_as<stdmath::float2>; })
		static auto y_axis(Tvector v) {
			return reaction::calc([](const auto& v) {
				return v.y;
			}, v);
		}

		template<reaction::IsReact Tvector>
		requires(requires (Tvector v) { {v()} -> std::same_as<stdmath::float2>; })
		static auto dead_zone(Tvector v) {
			return reaction::calc([](const auto& v, float dead_zone = .1) {
				if(length_squared(v) < dead_zone * dead_zone)
					return stdmath::float2{};
				return v;
			}, v);
		}

		template<reaction::IsReact Tvector>
		requires(requires (Tvector v) { {v()} -> std::same_as<stdmath::float2>; })
		static auto normalize(Tvector v) {
			return reaction::calc([](const auto& v) {
				return stdmath::normalize(v);
			}, v);
		}

		auto wasd(bool also_use_arrow_keys = true) {
			if(also_use_arrow_keys)
				return buttons_to_vector(
					any_of(get_boolean(key_w), get_boolean(key_up)), 
					any_of(get_boolean(key_s), get_boolean(key_down)), 
					any_of(get_boolean(key_a), get_boolean(key_left)), 
					any_of(get_boolean(key_d), get_boolean(key_right))
				);
			return buttons_to_vector(get_boolean(key_w), get_boolean(key_s), get_boolean(key_a), get_boolean(key_d));
		}

		template<reaction::IsReact... Tactions>
		static auto max(Tactions... actions) {
			return reaction::calc([](auto... vs) {
				return std::max(vs...);
			}, actions...);
		}

		template<reaction::IsReact... Tactions>
		static auto min(Tactions... actions) {
			return reaction::calc([](auto... vs) {
				return std::min(vs...);
			}, actions...);
		}

		template<reaction::IsReact... Tactions>
		static auto sum(Tactions... actions) {
			return reaction::expr((actions + ...));
		}
	};

	export template<typename T>
	concept input_concept = std::derived_from<T, input> && requires(T t) {
		{ t.type } -> std::convertible_to<size_t>;
	};

}

