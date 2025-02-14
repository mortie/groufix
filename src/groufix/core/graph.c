/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */

#include "groufix/core/objects.h"
#include <assert.h>


/****************************
 * Promptly purges (destructs) all passes, leaving the graph 'empty'.
 * @param renderer Cannot be NULL, its graph state must not be empty.
 */
static void _gfx_render_graph_purge(GFXRenderer* renderer)
{
	assert(renderer != NULL);
	assert(renderer->graph.state != _GFX_GRAPH_EMPTY);

	// Destruct all passes.
	for (size_t i = 0; i < renderer->graph.passes.size; ++i)
		_gfx_pass_destruct(
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i));

	// The graph is now empty.
	renderer->graph.state = _GFX_GRAPH_EMPTY;
}

/****************************
 * Analyzes the render graph to setup all passes for correct builds.
 * @param renderer Cannot be NULL, its graph state must not yet be validated.
 */
static bool _gfx_render_graph_analyze(GFXRenderer* renderer)
{
	assert(renderer != NULL);
	assert(renderer->graph.state < _GFX_GRAPH_VALIDATED);

	// TODO: Analyze the graph for e.g. pass merging, this should log errors!

	// Its now validated!
	renderer->graph.state = _GFX_GRAPH_VALIDATED;

	return 1;
}

/****************************/
void _gfx_render_graph_init(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	gfx_vec_init(&renderer->graph.sinks, sizeof(GFXPass*));
	gfx_vec_init(&renderer->graph.passes, sizeof(GFXPass*));

	// No graph is a valid graph.
	renderer->graph.state = _GFX_GRAPH_BUILT;
}

/****************************/
void _gfx_render_graph_clear(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	// Destroy all passes, we want to make sure we do not destroy any pass
	// before all passes that reference it are destroyed.
	// Luckily, all parents of a pass will be to its left due to
	// submission order, which is always honored.
	// So we manually destroy 'em all in reverse order :)
	for (size_t i = renderer->graph.passes.size; i > 0; --i)
		_gfx_destroy_pass(
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i-1));

	gfx_vec_clear(&renderer->graph.passes);
	gfx_vec_clear(&renderer->graph.sinks);
}

/****************************/
bool _gfx_render_graph_warmup(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	// Already done.
	if (renderer->graph.state >= _GFX_GRAPH_WARMED)
		return 1;

	// With the same logic as building; we purge all things first.
	if (renderer->graph.state == _GFX_GRAPH_INVALID)
		_gfx_render_graph_purge(renderer);

	// If not valid yet, analyze the graph.
	if (renderer->graph.state < _GFX_GRAPH_VALIDATED)
		if (!_gfx_render_graph_analyze(renderer))
			return 0;

	// And then make sure all passes are warmped up!
	size_t failed = 0;

	for (size_t i = 0; i < renderer->graph.passes.size; ++i)
	{
		// No need to worry about destructing, state remains 'validated'.
		failed += !_gfx_pass_warmup(
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i));
	}

	if (failed > 0)
	{
		gfx_log_error(
			"Failed to warmup %"GFX_PRIs" pass(es) of the renderer's graph.",
			failed);

		return 0;
	}

	// Not completely built, but it can be invalidated.
	renderer->graph.state = _GFX_GRAPH_WARMED;

	return 1;
}

/****************************/
bool _gfx_render_graph_build(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	// Already done.
	if (renderer->graph.state == _GFX_GRAPH_BUILT)
		return 1;

	// When the graph is not valid, it needs to be entirely rebuilt.
	// Optimizations such as merging passes may change,
	// we want to capture these changes.
	if (renderer->graph.state == _GFX_GRAPH_INVALID)
		_gfx_render_graph_purge(renderer);

	// If not valid yet, analyze the graph.
	if (renderer->graph.state < _GFX_GRAPH_VALIDATED)
		if (!_gfx_render_graph_analyze(renderer))
			return 0;

	// So now make sure all the passes in the graph are built.
	size_t failed = 0;

	for (size_t i = 0; i < renderer->graph.passes.size; ++i)
	{
		GFXPass* pass =
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i);

		// The pass itself should log errors.
		// No need to worry about destructing, state remains 'validated'.
		failed += !_gfx_pass_build(pass, 0);

		// At this point we also sneakedly set the order of all passes
		// so the recorders know what's up.
		pass->order = (unsigned int)i;
	}

	if (failed > 0)
	{
		gfx_log_error(
			"Failed to build %"GFX_PRIs" pass(es) of the renderer's graph.",
			failed);

		return 0;
	}

	// Yep it's built.
	renderer->graph.state = _GFX_GRAPH_BUILT;

	return 1;
}

/****************************/
void _gfx_render_graph_rebuild(GFXRenderer* renderer, size_t index,
                               _GFXRecreateFlags flags)
{
	assert(renderer != NULL);
	assert(flags & _GFX_RECREATE);

	// Nothing to rebuild if nothing is built.
	if (renderer->graph.state < _GFX_GRAPH_WARMED)
		return;

	// Loop over all passes and check if they read from or write to the
	// attachment index, if so, rebuild those passes.
	size_t failed = 0;

	for (size_t i = 0; i < renderer->graph.passes.size; ++i)
	{
		GFXPass* pass =
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i);

		// TODO: Also check if it's using it as an image attachment.
		// Check if it's writing to it as a window back-buffer.
		if (pass->build.backing == index)
		{
			// If we fail, just ignore and signal we're not built.
			// Will be tried again in _gfx_render_graph_build.
			failed += !_gfx_pass_build(pass, flags);
		}
	}

	if (failed > 0)
	{
		gfx_log_warn(
			"Failed to rebuild %"GFX_PRIs" pass(es) of the renderer's graph.",
			failed);

		// The graph is not invalid, but incomplete.
		renderer->graph.state = _GFX_GRAPH_VALIDATED;
	}
}

/****************************/
void _gfx_render_graph_destruct(GFXRenderer* renderer, size_t index)
{
	assert(renderer != NULL);

	// Loop over all passes and check if they read from or write to the
	// attachment index, if so, destruct the pass.
	for (size_t i = 0; i < renderer->graph.passes.size; ++i)
	{
		GFXPass* pass =
			*(GFXPass**)gfx_vec_at(&renderer->graph.passes, i);

		// TODO: Also check if it's using it as an image attachment.
		// Check if it's writing to it as a window back-buffer.
		if (pass->build.backing == index)
		{
			_gfx_pass_destruct(pass);

			// The graph is incomplete now.
			if (renderer->graph.state >= _GFX_GRAPH_WARMED)
				renderer->graph.state = _GFX_GRAPH_VALIDATED;
		}
	}
}

/****************************/
void _gfx_render_graph_invalidate(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	// Just set the flag, it is used to destruct everything at the start of
	// the next build call. This way we can re-analyze it.
	if (renderer->graph.state != _GFX_GRAPH_EMPTY)
		renderer->graph.state = _GFX_GRAPH_INVALID;
}

/****************************/
GFX_API GFXPass* gfx_renderer_add_pass(GFXRenderer* renderer,
                                       size_t numParents, GFXPass** parents)
{
	assert(renderer != NULL);
	assert(!renderer->recording);
	assert(numParents == 0 || parents != NULL);

	// Create a new pass.
	GFXPass* pass =
		_gfx_create_pass(renderer, numParents, parents);

	if (pass == NULL)
		goto error;

	// Add the new pass as a sink, as it has no 'children' yet.
	if (!gfx_vec_push(&renderer->graph.sinks, 1, &pass))
		goto clean;

	// Find the right place to insert the new pass at,
	// we pre-sort on level, this essentially makes it such that
	// every pass is submitted as early as possible.
	// Note that within a level, the adding order is preserved.
	// Backwards linear search is probably in-line with the adding order :p
	size_t loc;
	for (loc = renderer->graph.passes.size; loc > 0; --loc)
	{
		unsigned int level =
			(*(GFXPass**)gfx_vec_at(&renderer->graph.passes, loc-1))->level;

		if (level <= pass->level)
			break;
	}

	// Insert at found position.
	if (!gfx_vec_insert(&renderer->graph.passes, 1, &pass, loc))
	{
		gfx_vec_pop(&renderer->graph.sinks, 1);
		goto clean;
	}

	// Loop through all sinks, remove if it's now a parent.
	// Skip the last element, as we just added that.
	for (size_t t = renderer->graph.sinks.size-1; t > 0; --t)
	{
		GFXPass* sink =
			*(GFXPass**)gfx_vec_at(&renderer->graph.sinks, t-1);

		size_t d;
		for (d = 0; d < numParents; ++d)
			if (sink == parents[d]) break;

		if (d < numParents)
			gfx_vec_erase(&renderer->graph.sinks, 1, t-1);
	}

	// We added a pass, we need to re-analyze
	// because we may have new parent/child links.
	if (renderer->graph.state != _GFX_GRAPH_EMPTY)
		renderer->graph.state =
			// If the first pass, no need to purge, just set to empty.
			(renderer->graph.passes.size > 1) ?
				_GFX_GRAPH_INVALID : _GFX_GRAPH_EMPTY;

	return pass;


	// Cleanup on failure.
clean:
	_gfx_destroy_pass(pass);
error:
	gfx_log_error("Could not add a new pass to a renderer's graph.");

	return NULL;
}

/****************************/
GFX_API size_t gfx_renderer_get_num_sinks(GFXRenderer* renderer)
{
	assert(renderer != NULL);

	return renderer->graph.sinks.size;
}

/****************************/
GFX_API GFXPass* gfx_renderer_get_sink(GFXRenderer* renderer, size_t sink)
{
	assert(renderer != NULL);
	assert(sink < renderer->graph.sinks.size);

	return *(GFXPass**)gfx_vec_at(&renderer->graph.sinks, sink);
}
