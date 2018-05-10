#include "warnings.h"

#include "asar.h"
#include <assert.h>
#include <stdarg.h>

// This function should only be called inside this file now.
void warn(int errid, const char * e);

static int asar_num_warnings = 0;

struct asar_warning_mapping
{
	asar_warning_id warnid;
	const char* message;
	bool enabled;
	bool enabled_default;

	asar_warning_mapping(asar_warning_id warnid, const char* message, bool enabled = true)
	{
		++asar_num_warnings;

		this->warnid = warnid;
		this->message = message;
		this->enabled = enabled;
		this->enabled_default = enabled;

		// RPG Hacker: Sanity check. This makes sure that the order
		// of entries in asar_warnings matches with the order of
		// constants in asar_warning_id. This is important because
		// we access asar_warnings as an array.
		// Would love to do this via static_assert(), but can't
		// think of a way to do so.
		assert(warnid - warning_id_start == asar_num_warnings);
	}
};

// Keep in sync with asar_warning_id.
// Both, enum mapping and order, must match.
static asar_warning_mapping asar_warnings[] =
{
	{ warning_id_relative_path_used, "Relative %s path passed to asar_patch_ex() - please use absolute paths only to prevent undefined behavior!" },

	{ warning_id_rom_too_short, "ROM is too short to have a title. (Expected '%s')" },
	{ warning_id_rom_title_incorrect, "ROM title is incorrect. Expected '%s', got '%s'." },

	{ warning_id_65816_yy_x_does_not_exist, "($yy),x does not exist, assuming $yy,x." },
	{ warning_id_65816_xx_y_assume_16_bit, "%s $xx,y is not valid with 8-bit parameters, assuming 16-bit." },
	{ warning_id_spc700_assuming_8_bit, "This opcode does not exist with 16-bit parameters, assuming 8-bit." },

	{ warning_id_cross_platform_path, "This patch may not assemble cleanly on all platforms. Please use / instead." },

	{ warning_id_missing_org, "Missing org or freespace command." },
	{ warning_id_set_middle_byte, "It would be wise to set the 008000 bit of this address." },

	{ warning_id_unrecognized_special_command, "Unrecognized special command - your version of Asar might be outdated." },

	{ warning_id_freespace_leaked, "This freespace appears to be leaked." },

	{ warning_id_warn_command, "warn command%s" },

	{ warning_id_implicitly_sized_immediate, "Implicitly sized immediate.", false },

	{ warning_id_xkas_deprecated, "xkas support is being deprecated and will be removed in a future version of Asar. Please use an older version of Asar (<=1.50) if you need it." },
	{ warning_id_xkas_eat_parentheses, "xkas compatibility warning: Unlike xkas, Asar does not eat parentheses after defines." },
	{ warning_id_xkas_label_access, "xkas compatibility warning: Label access is always 24bit in emulation mode, but may be 16bit in native mode." },
	{ warning_id_xkas_warnpc_relaxed, "xkas conversion warning : warnpc is relaxed one byte in Asar." },
	{ warning_id_xkas_style_conditional, "xkas-style conditional compilation detected. Please use the if command instead." },
	{ warning_id_xkas_patch, "If you want to assemble an xkas patch, add ;@xkas at the top or you may run into a couple of problems." },
	{ warning_id_xkas_incsrc_relative, "xkas compatibility warning: incsrc and incbin look for files relative to the patch in Asar, but xkas looks relative to the assembler." },
	{ warning_id_convert_to_asar, "Convert the patch to native Asar format instead of making an Asar-only xkas patch." },

	{ warning_id_fixed_deprecated, "the 'fixed' parameter on freespace/freecode/freedata is deprecated - please use 'static' instead." },

	{ warning_id_autoclear_deprecated, "'autoclear' is deprecated - please use 'autoclean' instead." },
};

// RPG Hacker: Sanity check. This makes sure that the element count of asar_warnings
// matches with the number of constants in asar_warning_id. This is important, because
// we are going to access asar_warnings as an array.
static_assert(sizeof(asar_warnings) / sizeof(asar_warnings[0]) == warning_id_count, "asar_warnings and asar_warning_id are not in sync");

void asar_throw_warning(int whichpass, asar_warning_id warnid, ...)
{
	if (pass == whichpass)
	{
		assert(warnid > warning_id_start && warnid < warning_id_end);

		const asar_warning_mapping& warning = asar_warnings[warnid - warning_id_start - 1];

		if (warning.enabled)
		{
			char warning_buffer[1024];
			va_list args;
			va_start(args, warnid);
			vsnprintf(warning_buffer, sizeof(warning_buffer), warning.message, args);
			va_end(args);
			warn((int)warnid, warning_buffer);
		}
	}
}



void set_warning_enabled(asar_warning_id warnid, bool enabled)
{
	assert(warnid > warning_id_start && warnid < warning_id_end);

	asar_warning_mapping& warning = asar_warnings[warnid - warning_id_start - 1];

	warning.enabled = enabled;
}

asar_warning_id parse_warning_id_from_string(const char* string)
{
	const char* pos = string;

	if (pos == nullptr)
	{
		return warning_id_end;
	}

	if (pos[0] == 'w' || pos[0] == 'W')
	{
		++pos;
	}

	char* endpos = nullptr;
	int numid = (int)strtol(pos, &endpos, 10);

	if (endpos == nullptr || endpos[0] != '\0')
	{
		return warning_id_end;
	}

	asar_warning_id warnid = (asar_warning_id)numid;

	if (warnid <= warning_id_start || warnid >= warning_id_end)
	{
		return warning_id_end;
	}

	return warnid;
}

void reset_warnings_to_default()
{
	for (int i = (int)(warning_id_start + 1); i < (int)warning_id_end; ++i)
	{
		asar_warning_mapping& warning = asar_warnings[i - (int)warning_id_start - 1];

		warning.enabled = warning.enabled_default;
	}
}

struct warnings_state
{
	bool enabled[warning_id_count];
};

autoarray<warnings_state> warnings_state_stack;
warnings_state main_warnings_state;

void push_warnings(bool warnings_command)
{
	warnings_state current_state;

	for (int i = 0; i < (int)warning_id_count; ++i)
	{
		current_state.enabled[i] = asar_warnings[i].enabled;
	}

	if (warnings_command)
	{
		warnings_state_stack.append(current_state);
	}
	else
	{
		main_warnings_state = current_state;
	}
}

void pull_warnings(bool warnings_command)
{
	if (warnings_state_stack.count > 0 || !warnings_command)
	{
		warnings_state prev_state;

		if (warnings_command)
		{
			prev_state = warnings_state_stack[warnings_state_stack.count - 1];
		}
		else
		{
			prev_state = main_warnings_state;
		}

		for (int i = 0; i < (int)warning_id_count; ++i)
		{
			asar_warnings[i].enabled = prev_state.enabled[i];
		}

		if (warnings_command)
		{
			warnings_state_stack.remove(warnings_state_stack.count - 1);
		}
	}
	else
	{
		asar_throw_error(0, error_type_block, error_id_pullwarnings_without_pushwarnings);
	}
}

void verify_warnings()
{
	if (warnings_state_stack.count > 0)
	{
		asar_throw_error(0, error_type_null, error_id_pushwarnings_without_pullwarnings);
		
		warnings_state_stack.reset();
	}
}
