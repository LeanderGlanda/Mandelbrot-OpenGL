#version 430

uniform int width;
uniform int height;

uniform int maxIteration;
uniform double middlea;
uniform double middleb;
uniform double rangea;
uniform double rangeb;

layout (binding = 0, rgba16f) uniform image2D destTex; // Input the texture with the correct format
layout (local_size_x = 32, local_size_y = 32) in; // Specify the amount of calculation groups

void main()
{
    // Get the position one time as int vector and the other time as double vector
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    vec2 xy = vec2(gl_GlobalInvocationID.xy);

    // Calculate the mandelbrot
    double x0 = xy.x / width;
    double y0 = xy.y / height;

    x0 = (x0 * rangea) + middlea - (rangea / 2);
    y0 = (y0 * rangeb) + middleb - (rangeb / 2);

    double real = 0;
    double imaginary = 0;
    vec3 color;

    double iteration = 0;
    while (((real * real) + (imaginary * imaginary)) <= 4 && iteration < maxIteration)
    {
        double temp = (real * real) - (imaginary * imaginary) + x0;
        imaginary = (2 * real * imaginary) + y0;
        real = temp;
        iteration++;
    }

    // Calculate the colors
    // Color algorithm from my brother (https://github.com/Julian-Wollersberger/Apfelmannchen)
    int runde;
    double fraction;

    if (iteration == maxIteration)
    {
        color = vec3(1.0f, 1.0f, 1.0f);
    }
    else
    {
        iteration += 8;
        runde = 15;
        while (iteration >= runde)
            runde = (runde * 2) + 1;

        fraction = (iteration - (runde / 2)) / (runde / 2);

        if (fraction < 0)
            color = vec3(1.0f, 1.0f, 1.0f);
        else if (fraction < 1.0 / 6) color = vec3(1.0, 0.0, fraction * 6.0);
        else if (fraction < 2.0 / 6) color = vec3(1.0 - (fraction - 1.0 / 6.0) * 6.0, 0.0, 1.0);
        else if (fraction < 3.0 / 6) color = vec3(0.0, (fraction - 2.0 / 6.0) * 6.0, 1.0);
        else if (fraction < 4.0 / 6) color = vec3(0.0, 1.0, 1.0 - (fraction - 3.0 / 6.0) * 6.0);
        else if (fraction < 5.0 / 6) color = vec3((fraction - 4.0 / 6.0) * 6.0, 1.0, 0.0);
        else if (fraction <= 6.0 / 6) color = vec3(1.0, 1.0 - (fraction - 5.0 / 6.0) * 6.0f, 0.0);
        else
        {
            color = vec3(0.0f, 0.0f, 0.0f);
        }
    }

    // Store the calculated color in the current x, y position of the texture
    imageStore(destTex, storePos, vec4(color.x, color.y, color.z, 0.0f));
}