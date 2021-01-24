/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */

#include "test.h"


/****************************
 * Minimal test.
 */
TEST_DESCRIBE(minimal, _t)
{
	// Create a single render pass that writes to the window.
	GFXRenderPass* pass = gfx_renderer_add(_t->renderer, 0, NULL);
	if (pass == NULL)
		TEST_FAIL();

	if (!gfx_render_pass_write(pass, 0))
		TEST_FAIL();

	// Setup an event loop.
	// We wait instead of poll, only update when an event was detected.
	while (!gfx_window_should_close(_t->window))
	{
		gfx_renderer_submit(_t->renderer);
		gfx_wait_events();
	}
}


/****************************
 * Run the minimal test.
 */
TEST_MAIN(minimal);
