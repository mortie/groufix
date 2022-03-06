/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 *
 * This file contains the header-only testing utility.
 * Use the following macros to describe and run a test:
 *
 * TEST_DESCRIBE(name, base)
 *   Describe a new test, the syntax is similar to a function:
 *   TEST_DESCRIBE(basic_test, t) { gfx_window_set_title(t->window, "test"); }
 *   Where `t` is the name of the exposed TestBase pointer.
 *
 * TEST_FAIL()
 *   Forces the test to fail and exits the program.
 *
 * TEST_RUN(name)
 *   Call from within a test, run another test by name.
 *   Becomes a no-op if an instance of name is already running.
 *
 * TEST_RUN_THREAD(name)
 *   Same as TEST_RUN(name), except the test will run in a new thread.
 *   This will attach and detach the thread to and from groufix appropriately.
 *
 * TEST_JOIN(name)
 *   Joins a threaded test by name.
 *   Becomes a no-op if no threaded instance of name is running.
 *
 * TEST_MAIN(name)
 *   Main entry point of the program by test name, use as follows:
 *   TEST_MAIN(basic_test);
 *
 * To enable threading, TEST_ENABLE_THREADS must be defined. Threading is
 * implementeded using pthreads. The compiler should support this, luckily
 * Mingw-w64 does on all platforms so no issues on Windows.
 *
 * The testing utility initializes groufix and opens a window backed by a
 * default renderer setup. To override default behaviour you can disable some
 * building steps, define one of the following before includng this file:
 *
 * TEST_SKIP_EVENT_HANDLERS
 *   Do not register the default event handlers for the base window.
 *   To set event handlers yourself, default event handlers are defined:
 *    TEST_EVT_KEY_RELEASE
 *
 * TEST_SKIP_CREATE_RENDER_GRAPH
 *   Do not build a render graph,
 *   i.e. no passes are added to the base renderer.
 */


#ifndef TEST_H
#define TEST_H

#include <groufix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (TEST_ENABLE_THREADS)
	#include <pthread.h>
#endif


// Describes a test function that can be called.
#define TEST_DESCRIBE(name, base) \
	static TestState _test_state_##name = { .state = TEST_IDLE }; \
	void _test_func_##name(TestBase* base)

// Forces the test to fail.
#define TEST_FAIL() \
	_test_fail()

// Runs a test function from within another test function.
#define TEST_RUN(name) \
	do { \
		if (_test_state_##name.state == TEST_IDLE) { \
			_test_state_##name.state = TEST_RUNNING; \
			_test_func_##name(&_test_base) \
			_test_state_##name.state = TEST_IDLE; \
		} \
	} while (0)

// Runs a test in a new thread.
#define TEST_RUN_THREAD(name) \
	do { \
		if (_test_state_##name.state == TEST_IDLE) { \
			_test_state_##name.state = TEST_RUNNING_THRD; \
			_test_state_##name.f = _test_func_##name; \
			if (pthread_create(&_test_state_##name.thrd, NULL, _test_thrd, &_test_state_##name)) \
				TEST_FAIL(); \
		} \
	} while (0)

// Joins a threaded test function.
#define TEST_JOIN(name) \
	do { \
		if (_test_state_##name.state == TEST_RUNNING_THRD) { \
			void* _test_ret; \
			pthread_join(_test_state_##name.thrd, &_test_ret); \
			_test_state_##name.state = TEST_IDLE; \
		} \
	} while(0)

// Main entry point for a test program, runs the given test name.
#define TEST_MAIN(name) \
	int main(void) { \
		_test_init(); \
		_test_state_##name.state = TEST_RUNNING; \
		_test_func_##name(&_test_base); \
		_test_state_##name.state = TEST_IDLE; \
		_test_end(); \
	} \
	int _test_unused_for_semicolon


/**
 * Default event handlers.
 */
#define TEST_EVT_KEY_RELEASE _test_key_release


/**
 * Base testing state, modify at your leisure :)
 */
typedef struct
{
	GFXDevice*     device;
	GFXWindow*     window;
	GFXHeap*       heap;
	GFXDependency* dep;
	GFXRenderer*   renderer; // Window is attached at index 0.
	GFXPrimitive*  primitive;
	GFXGroup*      group;

} TestBase;


/**
 * Thread handle.
 */
typedef struct
{
	enum
	{
		TEST_IDLE,
		TEST_RUNNING,
		TEST_RUNNING_THRD

	} state;

#if defined (TEST_ENABLE_THREADS)
	void (*f)(TestBase*);
	pthread_t thrd;
#endif

} TestState;


/**
 * Instance of the test base state.
 */
static TestBase _test_base =
{
	.device = NULL,
	.window = NULL,
	.heap = NULL,
	.dep = NULL,
	.renderer = NULL,
	.primitive = NULL,
	.group = NULL
};


/****************************
 * All internal testing functions.
 ****************************/

/**
 * Default key release event handler.
 */
static void _test_key_release(GFXWindow* window,
                              GFXKey key, int scan, GFXModifier mod)
{
	switch (key)
	{
	// Toggle fullscreen on F11.
	case GFX_KEY_F11:
		if (gfx_window_get_monitor(window) != NULL)
		{
			gfx_window_set_monitor(
				window, NULL,
				(GFXVideoMode){ .width = 600, .height = 400 });
		}
		else
		{
			GFXMonitor* monitor = gfx_get_primary_monitor();
			gfx_window_set_monitor(
				window, monitor,
				gfx_monitor_get_current_mode(monitor));
		}
		break;

	// Close on escape.
	case GFX_KEY_ESCAPE:
		gfx_window_set_close(window, 1);
		break;

	default:
		break;
	}
}

/**
 * Clears the base test state.
 */
static void _test_clear(void)
{
	gfx_destroy_renderer(_test_base.renderer);
	gfx_destroy_heap(_test_base.heap);
	gfx_destroy_dep(_test_base.dep);
	gfx_destroy_window(_test_base.window);
	gfx_terminate();

	// Don't bother resetting _test_base as we will exit() anyway.
}

/**
 * Forces the test to fail and exits the program.
 */
static void _test_fail(void)
{
	_test_clear();

	fputs("\n* TEST FAILED\n", stderr);
	exit(EXIT_FAILURE);
}

/**
 * End (i.e. exit) the test program.
 */
static void _test_end(void)
{
	_test_clear();

	fputs("\n* TEST SUCCESSFUL\n", stderr);
	exit(EXIT_SUCCESS);
}


#if defined (TEST_ENABLE_THREADS)

/**
 * Thread entry point for a test.
 */
static void* _test_thrd(void* arg)
{
	TestState* test = arg;

	if (!gfx_attach())
		TEST_FAIL();

	test->f(&_test_base);
	gfx_detach();

	return NULL;
}

#endif


/**
 * Initializes the test base program.
 */
static void _test_init(void)
{
	// Initialize.
	if (!gfx_init())
		TEST_FAIL();

	// Create a window.
	_test_base.window = gfx_create_window(
		GFX_WINDOW_RESIZABLE | GFX_WINDOW_DOUBLE_BUFFER,
		_test_base.device, NULL,
		(GFXVideoMode){ .width = 600, .height = 400 }, "groufix");

	if (_test_base.window == NULL)
		TEST_FAIL();

#if !defined (TEST_SKIP_EVENT_HANDLERS)
	// Register the default key events.
	_test_base.window->events.key.release = TEST_EVT_KEY_RELEASE;
#endif

	// Create a heap & dependency.
	_test_base.heap = gfx_create_heap(_test_base.device);
	if (_test_base.heap == NULL)
		TEST_FAIL();

	_test_base.dep = gfx_create_dep(_test_base.device);
	if (_test_base.dep == NULL)
		TEST_FAIL();

	// Create a renderer and attach the window at index 0.
	_test_base.renderer = gfx_create_renderer(_test_base.device, 2);
	if (_test_base.renderer == NULL)
		TEST_FAIL();

	if (!gfx_renderer_attach_window(_test_base.renderer, 0, _test_base.window))
		TEST_FAIL();

#if !defined (TEST_SKIP_CREATE_RENDER_GRAPH)
	// Allocate a primitive.
	uint16_t indexData[] = {
		0, 1, 3, 2
	};

	float vertexData[] = {
		-0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
	};

	_test_base.primitive = gfx_alloc_prim(_test_base.heap,
		GFX_MEMORY_WRITE, 0,
		GFX_TOPO_TRIANGLE_STRIP,
		4, sizeof(uint16_t), 4,
		GFX_REF_NULL,
		3, (GFXAttribute[]){
			{
				.format = GFX_FORMAT_R32G32B32_SFLOAT,
				.offset = 0,
				.stride = sizeof(float) * 8,
				.buffer = GFX_REF_NULL
			}, {
				.format = GFX_FORMAT_R32G32B32_SFLOAT,
				.offset = sizeof(float) * 3,
				.stride = sizeof(float) * 8,
				.buffer = GFX_REF_NULL
			}, {
				.format = GFX_FORMAT_R32G32_SFLOAT,
				.offset = sizeof(float) * 6,
				.stride = sizeof(float) * 8,
				.buffer = GFX_REF_NULL
			}
		});

	if (_test_base.primitive == NULL)
		TEST_FAIL();

	GFXBufferRef vert = gfx_ref_prim_vertices(_test_base.primitive, 0);
	GFXBufferRef ind = gfx_ref_prim_indices(_test_base.primitive);

	if (!gfx_write(vertexData, vert, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{ .offset = 0, .size = sizeof(vertexData) }},
		(GFXRegion[]){{ .offset = 0, .size = 0 }},
		(GFXInject[]){
			gfx_dep_sig(_test_base.dep, GFX_ACCESS_VERTEX_READ, 0)
		}))
	{
		TEST_FAIL();
	}

	if (!gfx_write(indexData, ind, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{ .offset = 0, .size = sizeof(indexData) }},
		(GFXRegion[]){{ .offset = 0, .size = 0 }},
		(GFXInject[]){
			gfx_dep_sig(_test_base.dep, GFX_ACCESS_INDEX_READ, 0)
		}))
	{
		TEST_FAIL();
	}

	// Allocate a group with an mvp matrix and a texture.
	float uboData[] = {
		1.0f, 0.2f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	uint8_t imgData[] = {
		255, 0, 255, 0,
		0, 255, 0, 255,
		255, 0, 255, 0,
		0, 255, 0, 255
	};

	GFXImage* image = gfx_alloc_image(_test_base.heap,
		GFX_MEMORY_WRITE, GFX_IMAGE_2D,
		GFX_IMAGE_SAMPLED, GFX_FORMAT_R8_UNORM, 1, 1,
		4, 4, 1);

	if (image == NULL)
		TEST_FAIL();

	_test_base.group = gfx_alloc_group(_test_base.heap,
		GFX_MEMORY_WRITE,
		GFX_BUFFER_UNIFORM,
		2, (GFXBinding[]){
			{
				.type = GFX_BINDING_BUFFER,
				.count = 1,
				.elementSize = sizeof(float) * 16,
				.numElements = 1,
				.buffers = NULL
			}, {
				.type = GFX_BINDING_IMAGE,
				.count = 1,
				.images = (GFXImageRef[]){ gfx_ref_image(image) }
			}
		});

	if (_test_base.group == NULL)
		TEST_FAIL();

	GFXBufferRef ubo = gfx_ref_group_buffer(_test_base.group, 0, 0);
	GFXImageRef img = gfx_ref_group_image(_test_base.group, 1, 0);

	if (!gfx_write(uboData, ubo, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{ .offset = 0, .size = sizeof(uboData) }},
		(GFXRegion[]){{ .offset = 0, .size = 0 }},
		(GFXInject[]){
			gfx_dep_sig(_test_base.dep,
				GFX_ACCESS_UNIFORM_READ, GFX_STAGE_VERTEX)
		}))
	{
		TEST_FAIL();
	}

	if (!gfx_write(imgData, img, GFX_TRANSFER_ASYNC, 1, 1,
		(GFXRegion[]){{
			.offset = 0,
			.rowSize = 0,
			.numRows = 0
		}},
		(GFXRegion[]){{
			.aspect = GFX_IMAGE_COLOR,
			.mipmap = 0, .layer = 0,  .numLayers = 1,
			.x = 0,      .y = 0,      .z = 0,
			.width = 4,  .height = 4, .depth = 1
		}},
		(GFXInject[]){
			gfx_dep_sig(_test_base.dep,
				GFX_ACCESS_SAMPLED_READ, GFX_STAGE_FRAGMENT)
		}))
	{
		TEST_FAIL();
	}

	// Add a single pass that writes to the window.
	GFXPass* pass = gfx_renderer_add_pass(_test_base.renderer, 0, NULL);
	if (pass == NULL)
		TEST_FAIL();

	if (!gfx_pass_consume(pass, 0, GFX_ACCESS_ATTACHMENT_WRITE, 0))
		TEST_FAIL();

	// Make it render the thing.
	gfx_pass_use(pass, _test_base.primitive, _test_base.group);
#endif
}


#endif
