// frosted glass: blurs what was drawn before it (a copy of the frame so far, bound as the texture) and tints it
// color_a tint (alpha = how much of it), params: x rounding, y radius in pixels, zw one over the framebuffer size

#include "effect.hlsli"

#define SAMPLES 48

float4 main(ps_input input) : SV_Target
{
    float2 texel = effect.params.zw;
    float2 center = input.pos.xy * texel;
    float4 sum = 0.0;
    float weight_sum = 0.0;

    [unroll]
    for (int i = 0; i < SAMPLES; i++) { // points spread evenly over a disc (golden angle spiral), weighted towards the middle
        float r = sqrt((i + 0.5) / SAMPLES);
        float angle = i * 2.39996323;
        float2 offset = float2(cos(angle), sin(angle)) * r * effect.params.y;
        float weight = exp(-2.0 * r * r);
        float2 uv = clamp(center + offset * texel, texel * 0.5, 1.0 - texel * 0.5); // stay inside the frame
        sum += source_texture.SampleLevel(source_sampler, uv, 0) * weight;
        weight_sum += weight;
    }

    float4 blurred = sum / weight_sum;
    float3 color = lerp(blurred.rgb, effect.color_a.rgb, effect.color_a.a) + dither(input.pos.xy);
    float alpha = max(blurred.a, effect.color_a.a); // keeps a see-through window see-through
    return float4(color, alpha * coverage(rounded_rect_distance(input.pos.xy, effect.rect, effect.params.x)));
}
