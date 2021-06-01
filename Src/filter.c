#include "../Inc/filter.h"

int init_filter()
{
    AVFilterContext *text_filter;
    AVFilterInOut *inputs, *outputs;

    vfilter_ctx.filter_graph = NULL;
    vfilter_ctx.src_ctx = NULL;
    vfilter_ctx.sink_ctx = NULL;

    char* text_filter_str = "drawtext=text=Nohsihyun:x=100:y=100:fontsize=40:fontcolor=black";
 
    vfilter_ctx.filter_graph = avfilter_graph_alloc();
    if(vfilter_ctx.filter_graph == NULL)
        return -1;

    if(avfilter_graph_parse2(vfilter_ctx.filter_graph, "null", &inputs, &outputs)<0)
    {
        printf("Failed to parse video filtergraph\n");
        return -2;
    }

    if(avfilter_graph_create_filter(
        &vfilter_ctx.src_ctx, 
        avfilter_get_by_name("buffer"), 
        "in", text_filter_str, NULL, 
        vfilter_ctx.filter_graph)<0)
    {
        printf("Failed to create video buffer source\n");
        return -3;
    }    

    if(avfilter_link(vfilter_ctx.src_ctx, 0, inputs->filter_ctx, 0) < 0)
    {
        printf("Failed to link video buffer source\n");
        return -4;
    }

    // Create output filter
    // Create Buffer Sink
    if(avfilter_graph_create_filter(
          &vfilter_ctx.sink_ctx
          , avfilter_get_by_name("buffersink")
          , "out", NULL, NULL, vfilter_ctx.filter_graph) < 0)
    {
        printf("Failed to create video buffer sink\n");
        return -3;
    }

    // Configure all prepared filters.
    if(avfilter_graph_config(vfilter_ctx.filter_graph, NULL) < 0)
    {
        printf("Failed to configure video filter context\n");
        return -5;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}