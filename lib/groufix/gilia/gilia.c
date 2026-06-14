#include "groufix/gilia/gilia.h"

#ifndef GFX_HAS_GILIA
bool gfx_has_gilia(void) { return false; }
bool gfx_gilia_execute(const char *str) { return false; }
bool gfx_gilia_execute_file(FILE *f) { return false; }
#else

#include "groufix.h"

#include <gilia/parse/lex.h>
#include <gilia/parse/parse.h>
#include <gilia/gen/gen.h>
#include <gilia/module.h>
#include <gilia/modules/builtins.h>
#include <gilia/vm/vm.h>

// Convenience macro to define a method on a C object.
// It takes care of checking whether the callee is the right type or not.
#define DEFINE_METHOD(name, ctype_name) \
	static gil_word name ## _impl( \
		struct gil_vm *vm, gil_word mid, gil_word self, \
		gil_word argc, gil_word *argv); \
	static gil_word name( \
		struct gil_vm *vm, gil_word mid, gil_word self, \
		gil_word argc, gil_word *argv) \
	{ \
		GFXGiliaModule *mod = (GFXGiliaModule *)vm->cmodules[mid].mod; \
		struct gil_vm_value *selfval = &vm->values[self]; \
		if (gil_value_get_type(selfval) != GIL_VAL_TYPE_CVAL) { \
			return 0; \
		} \
		if (selfval->cval.ctype != mod->ctype_name) { \
			return 0; \
		} \
		if (!selfval->cval.cval) { \
			return 0; \
		} \
		return name ## _impl(vm, mid, self, argc, argv); \
	} \
	static gil_word name ## _impl( \
		struct gil_vm *vm, gil_word mid, gil_word self, \
		gil_word argc, gil_word *argv)

// Convenience macro to access the current object in a method
#define SELF (vm->values[self].cval.cval)

typedef struct GFXGiliaModule_ {
	struct gil_module base;

	gil_word
		kinit, kterminate, kdestroy, kcreatewindow,
		kpollevents, kshouldclose;
	gil_word twindow;
	gil_word nswindow;
} GFXGiliaModule;

static gil_word groufix_init(
	struct gil_vm *vm, gil_word mid, gil_word self,
	gil_word argc, gil_word *argv)
{
	if (!gfx_init()) {
		return gil_vm_error(vm, "Failed to initialize groufix");
	}

	return 0;
}

static gil_word groufix_terminate(
	struct gil_vm *vm, gil_word mid, gil_word self,
	gil_word argc, gil_word *argv)
{
	gfx_terminate();
	return 0;
}

static gil_word groufix_poll_events(
	struct gil_vm *vm, gil_word mid, gil_word self,
	gil_word argc, gil_word *argv)
{
	gfx_poll_events();
	return 0;
}

static gil_word groufix_create_window(
	struct gil_vm *vm, gil_word mid, gil_word self,
	gil_word argc, gil_word *argv)
{
	GFXGiliaModule *mod = (GFXGiliaModule *)vm->cmodules[mid].mod;
	GFXWindow *win = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER,
		NULL, NULL,
		(GFXVideoMode){ 600, 400, 0 }, "groufix");
	return gil_vm_make_cval(vm, mod->twindow, mod->nswindow, win);
	return 0;
}

DEFINE_METHOD(groufix_window_destroy, twindow)
{
	gfx_destroy_window(SELF);
	SELF = NULL;
	return 0;
}

DEFINE_METHOD(groufix_window_should_close, twindow)
{
	if (gfx_window_should_close(SELF)) {
		return vm->ktrue;
	} else {
		return vm->kfalse;
	}
	return 0;
}

static void module_init(
	struct gil_module *ptr,
	gil_word (*alloc)(void *data, const char *name), void *data)
{
	GFXGiliaModule *mod = (GFXGiliaModule *)ptr;
	mod->kinit = alloc(data, "init");
	mod->kterminate = alloc(data, "terminate");
	mod->kcreatewindow = alloc(data, "create-window");
	mod->kdestroy = alloc(data, "destroy");
	mod->kpollevents = alloc(data, "poll-events");
	mod->kshouldclose = alloc(data, "should-close");
}

static gil_word module_create(
	struct gil_module *ptr, struct gil_vm *vm, gil_word mid)
{
	GFXGiliaModule *mod = (GFXGiliaModule *)ptr;

	mod->twindow = gil_vm_alloc_ctype(vm);
	mod->nswindow = gil_vm_alloc(vm, GIL_VAL_TYPE_NAMESPACE, 0);
	struct gil_vm_value *nswindow = &vm->values[mod->nswindow];
	nswindow->ns.parent = 0;
	nswindow->ns.ns = NULL;
	gil_vm_namespace_set(
		nswindow, mod->kdestroy,
		gil_vm_make_cfunction(vm, groufix_window_destroy, mid));
	gil_vm_namespace_set(
		nswindow, mod->kshouldclose,
		gil_vm_make_cfunction(vm, groufix_window_should_close, mid));

	gil_word id = gil_vm_alloc(vm, GIL_VAL_TYPE_NAMESPACE, 0);
	struct gil_vm_value *ns = &vm->values[id];
	ns->ns.parent = 0;
	ns->ns.ns = NULL;
	gil_vm_namespace_set(
		ns, mod->kinit,
		gil_vm_make_cfunction(vm, groufix_init, mid));
	gil_vm_namespace_set(
		ns, mod->kterminate,
		gil_vm_make_cfunction(vm, groufix_terminate, mid));
	gil_vm_namespace_set(
		ns, mod->kcreatewindow,
		gil_vm_make_cfunction(vm, groufix_create_window, mid));
	gil_vm_namespace_set(
		ns, mod->kpollevents,
		gil_vm_make_cfunction(vm, groufix_poll_events, mid));

	return id;
}

static void module_marker(
		struct gil_module *ptr, struct gil_vm *vm,
		void (*mark)(struct gil_vm *vm, gil_word id))
{
	GFXGiliaModule *mod = (GFXGiliaModule *)ptr;
	mark(vm, mod->nswindow);
}

static void gfx_gilia_module_init(GFXGiliaModule *mod)
{
	mod->base.name = "groufix";
	mod->base.init = module_init;
	mod->base.create = module_create;
	mod->base.marker = module_marker;
}

bool gfx_has_gilia(void) { return true; }

static bool execute_reader(struct gil_io_reader *input)
{
	struct gil_mod_builtins mod_builtins;
	gil_mod_builtins_init(&mod_builtins);

	GFXGiliaModule mod_groufix;
	gfx_gilia_module_init(&mod_groufix);

	// Init lexer with its input reader
	struct gil_lexer lexer;
	gil_lexer_init(&lexer, input);

	// Init bytecode generator with its output writer
	struct gil_io_mem_writer bytecode_writer = {0};
	bytecode_writer.w.write = gil_io_mem_write;
	struct gil_generator gen;
	gil_gen_init(&gen, &bytecode_writer.w, &mod_builtins.base, NULL);
	gil_gen_register_module(&gen, &mod_groufix.base);

	// Parse and generate bytecode
	struct gil_parse_error err;
	struct gil_parse_context ctx = {&lexer, &gen, &err};
	if (gil_parse_program(&ctx) < 0) {
		fprintf(
			stderr, "Parse error: %i:%i: %s\n",
			err.line, err.ch, err.message);
		gil_parse_error_free(&err);
		gil_gen_free(&gen);
		free(bytecode_writer.mem);
		return false;
	}
	unsigned char *bytecode = bytecode_writer.mem;
	size_t bytecode_len = bytecode_writer.len;

	// Apply relocations
	for (gil_word i = 0; i < gen.relocslen; ++i) {
		gil_word pos = gen.relocs[i].pos;
		gil_word rep = gen.relocs[i].replacement;
		gil_gen_fixup_reloc(&bytecode[pos], rep);
	}

	gil_gen_free(&gen);

	// Execute bytecode
	struct gil_vm vm;
	gil_vm_init(&vm, bytecode, bytecode_len, &mod_builtins.base);
	gil_vm_register_module(&vm, &mod_groufix.base);
	gil_vm_run(&vm);
	free(bytecode);

	return true;
}

bool gfx_gilia_execute(const char *str)
{
	struct gil_io_mem_reader r = {0};
	r.r.read = gil_io_mem_read;
	r.mem = str;
	r.len = strlen(str);
	return execute_reader(&r.r);
}

bool gfx_gilia_execute_file(FILE *f)
{
	struct gil_io_file_reader r = {0};
	r.r.read = gil_io_file_read;
	r.f = f;
	return execute_reader(&r.r);
}

#endif
