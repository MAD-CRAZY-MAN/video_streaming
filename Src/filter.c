#include "../Inc/filter.h"

int init_filter()
{
    static int count = 0;
    count++;
    AVFilterContext *text_filter;
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVFilterInOut *outputs = avfilter_inout_alloc();
    char args[512];
    vfilter_ctx.filter_graph = NULL;
    vfilter_ctx.src_ctx = NULL;
    vfilter_ctx.sink_ctx = NULL;

   // char* text_filter_str = "drawtext=text='Nohsihyun':x=100:y=100:fontsize=40:fontcolor=black";
    char text_filter_str[512];
    snprintf(text_filter_str, sizeof(text_filter_str), "drawtext=text=%d:x=100:y=100:fontsize=40:fontcolor=black", count);
    snprintf(args, sizeof(args), "time_base=%d/%d:video_size=%dx%d:pix_fmt=%d:pixel_aspect=%d/%d"
    , deviceInput.stream->time_base.num, deviceInput.stream->time_base.den
    , deviceInput.codec_ctx->width, deviceInput.codec_ctx->height
    , deviceInput.codec_ctx->pix_fmt
    , deviceInput.codec_ctx->sample_aspect_ratio.num, deviceInput.codec_ctx->sample_aspect_ratio.den);

    vfilter_ctx.filter_graph = avfilter_graph_alloc();
    if(vfilter_ctx.filter_graph == NULL)
        return -1;

    // if(avfilter_graph_parse2(vfilter_ctx.filter_graph, "null", &inputs, &outputs)<0)
    // {
    //     printf("Failed to parse video filtergraph\n");
    //     return -2;
    // }

    // Create Buffer Source
    if(avfilter_graph_create_filter(
        &vfilter_ctx.src_ctx, 
        avfilter_get_by_name("buffer"), 
        "in", args, NULL, vfilter_ctx.filter_graph)<0)
    {
        printf("Failed to create video buffer source\n");
        return -3;
    }    

    // Create output filter
    // Create Buffer Sink
    if(avfilter_graph_create_filter(
          &vfilter_ctx.sink_ctx,
          avfilter_get_by_name("buffersink"),
          "out", NULL, NULL, vfilter_ctx.filter_graph) < 0)
    {
        printf("Failed to create video buffer sink\n");
        return -3;
    }
    outputs->name       = av_strdup("in");
	outputs->filter_ctx = vfilter_ctx.src_ctx;
	outputs->pad_idx    = 0;
	outputs->next       = NULL;

	inputs->name       = av_strdup("out");
	inputs->filter_ctx = vfilter_ctx.sink_ctx;
	inputs->pad_idx    = 0;
	inputs->next       = NULL;
    if(avfilter_graph_parse_ptr(vfilter_ctx.filter_graph, text_filter_str, &inputs, &outputs, NULL)<0)
    {
        printf("failed avfilter_graph_parse_ptr");
        return -1;
    }
    //     //Link Buffer Source with input filter
    // if(avfilter_link(vfilter_ctx.src_ctx, 0, inputs->filter_ctx, 0) < 0)
    // {
    //     printf("Failed to link video buffer source\n");
    //     return -4;
    // }

    // //Create Drawtext filter
    //  if(avfilter_graph_create_filter(
    //        &text_filter,
    //        avfilter_get_by_name("buffer"),
    //        "text", text_filter_str, NULL, vfilter_ctx.filter_graph) < 0)
    //  {
    //      printf("Failed to create video text filter\n");
    //      return -4;
    //  }
    
    // link drawtext filter with aformat filter
    // if(avfilter_link(outputs->filter_ctx, 0, text_filter, 0) < 0)
    // {
    //     printf("Failed to link video format filter1\n");
    //     return -4;
    // }

    // // aformat is linked with Buffer Sink filter.
    // if(avfilter_link(text_filter, 0, vfilter_ctx.sink_ctx, 0) < 0)
    // {
    //     printf("Failed to link video format filter2\n");
    //     return -4;
    // }

    // Configure all prepared filters.
    if(avfilter_graph_config(vfilter_ctx.filter_graph, NULL) < 0)
    {
        printf("Failed to configure video filter context\n");
        return -5;
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}