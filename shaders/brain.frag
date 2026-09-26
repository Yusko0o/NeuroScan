#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPosition;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
    mat4 mvp;

    vec4 renderSettings;
} pc;

float hash31(vec3 p)
{
    p =
        fract(
            p *
            0.1031
        );

    p +=
        dot(
            p,
            p.yzx +
            33.33
        );

    return fract(
        (
            p.x +
            p.y
        ) *
        p.z
    );
}

float activityField(
    vec3 p,
    float time
)
{
    float field =
        0.0;

    vec3 node1 =
        vec3(
            0.55,
            0.22,
            0.25
        );

    vec3 node2 =
        vec3(
            -0.38,
            0.48,
            -0.15
        );

    vec3 node3 =
        vec3(
            0.18,
            -0.42,
            0.48
        );

    vec3 node4 =
        vec3(
            -0.52,
            -0.10,
            0.22
        );

    float d1 =
        length(
            p -
            node1
        );

    float d2 =
        length(
            p -
            node2
        );

    float d3 =
        length(
            p -
            node3
        );

    float d4 =
        length(
            p -
            node4
        );

    float pulse1 =
        0.5 +
        0.5 *
        sin(
            time *
            2.4
        );

    float pulse2 =
        0.5 +
        0.5 *
        sin(
            time *
            1.8 +
            1.7
        );

    float pulse3 =
        0.5 +
        0.5 *
        sin(
            time *
            3.1 +
            3.4
        );

    float pulse4 =
        0.5 +
        0.5 *
        sin(
            time *
            2.0 +
            5.0
        );

    field +=
        exp(
            -d1 *
            d1 *
            42.0
        ) *
        pulse1;

    field +=
        exp(
            -d2 *
            d2 *
            38.0
        ) *
        pulse2;

    field +=
        exp(
            -d3 *
            d3 *
            45.0
        ) *
        pulse3;

    field +=
        exp(
            -d4 *
            d4 *
            40.0
        ) *
        pulse4;

    return clamp(
        field,
        0.0,
        1.0
    );
}

void main()
{
    float opacity =
        clamp(
            pc.renderSettings.x,
            0.02,
            1.0
        );

    float brightness =
        clamp(
            pc.renderSettings.y,
            0.0,
            1.0
        );

    float activityEnabled =
        pc.renderSettings.z;

    float time =
        pc.renderSettings.w;

    vec3 normal =
        normalize(
            fragNormal
        );

    vec3 viewDirection =
        normalize(
            -fragPosition +
            vec3(
                0.0,
                0.0,
                2.0
            )
        );

    vec3 lightDirection1 =
        normalize(
            vec3(
                -0.45,
                0.72,
                0.65
            )
        );

    vec3 lightDirection2 =
        normalize(
            vec3(
                0.65,
                -0.15,
                0.40
            )
        );

    float diffuse1 =
        max(
            dot(
                normal,
                lightDirection1
            ),
            0.0
        );

    float diffuse2 =
        max(
            dot(
                normal,
                lightDirection2
            ),
            0.0
        );

    float fresnel =
        pow(
            1.0 -
            clamp(
                abs(
                    dot(
                        normal,
                        viewDirection
                    )
                ),
                0.0,
                1.0
            ),
            2.4
        );

    float surfaceDetail =
        0.5 +
        0.5 *
        sin(
            fragPosition.x *
            31.0 +
            sin(
                fragPosition.y *
                28.0
            ) +
            fragPosition.z *
            35.0
        );

    surfaceDetail =
        smoothstep(
            0.25,
            0.85,
            surfaceDetail
        );

    vec3 deepColor =
        vec3(
            0.004,
            0.020,
            0.045
        );

    vec3 cortexBlue =
        vec3(
            0.015,
            0.28,
            0.62
        );

    vec3 electricBlue =
        vec3(
            0.02,
            0.58,
            1.00
        );

    vec3 cyan =
        vec3(
            0.08,
            0.92,
            1.00
        );

    vec3 violet =
        vec3(
            0.55,
            0.16,
            1.00
        );

    vec3 magenta =
        vec3(
            1.00,
            0.08,
            0.58
        );

    float lighting =
        diffuse1 *
        0.72 +
        diffuse2 *
        0.20;

    vec3 color =
        deepColor;

    color +=
        cortexBlue *
        (
            0.20 +
            lighting *
            0.85
        );

    color +=
        electricBlue *
        fresnel *
        0.80;

    color +=
        cyan *
        surfaceDetail *
        0.08;

    float activity =
        activityField(
            fragPosition,
            time
        ) *
        activityEnabled;

    float activityPulse =
        0.72 +
        0.28 *
        sin(
            time *
            4.0 +
            fragPosition.x *
            8.0
        );

    activity *=
        activityPulse;

    vec3 activityColor =
        mix(
            violet,
            magenta,
            smoothstep(
                0.25,
                0.90,
                activity
            )
        );

    color +=
        activityColor *
        activity *
        (
            0.7 +
            brightness *
            2.2
        );

    color +=
        cyan *
        fresnel *
        activity *
        0.65;

    float randomSpark =
        hash31(
            floor(
                fragPosition *
                22.0
            )
        );

    float sparkMask =
        step(
            0.975,
            randomSpark
        );

    float sparkPulse =
        0.5 +
        0.5 *
        sin(
            time *
            6.0 +
            randomSpark *
            40.0
        );

    color +=
        cyan *
        sparkMask *
        sparkPulse *
        activityEnabled *
        brightness *
        0.75;

    color *=
        0.65 +
        brightness *
        1.20;

    color =
        color /
        (
            color +
            vec3(
                1.0
            )
        );

    color =
        pow(
            color,
            vec3(
                0.86
            )
        );

    float finalAlpha =
        opacity;

    finalAlpha +=
        fresnel *
        0.08;

    finalAlpha +=
        activity *
        0.08;

    finalAlpha =
        clamp(
            finalAlpha,
            0.03,
            1.0
        );

    outColor =
        vec4(
            color,
            finalAlpha
        );
}