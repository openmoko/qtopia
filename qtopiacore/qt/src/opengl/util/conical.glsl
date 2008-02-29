// conical gradient shader
#define M_PI  3.14159265358979323846
uniform sampler1D palette;
uniform float angle;
uniform vec4 inv_matrix;
uniform vec2 inv_matrix_offset;

void main()
{
    mat2 mat;
    mat[0][0] = inv_matrix.x;
    mat[0][1] = inv_matrix.y;
    mat[1][0] = inv_matrix.z;
    mat[1][1] = inv_matrix.w;

    vec2 A = gl_FragCoord.xy * mat + inv_matrix_offset;
/*     float val = fmod((atan2(-A.y, A.x) + angle) / (2.0 * M_PI), 1); */
    if (abs(A.y) == abs(A.x))
 	A.y += 0.002;
    float t = (atan2(-A.y, A.x) + angle) / (2.0 * M_PI);
    float val = t - floor(t);
    gl_FragColor = texture1D(palette, val);
}
