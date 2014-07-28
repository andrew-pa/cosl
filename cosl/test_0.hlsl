#version 400 core
~~~~
#ifndef IN_FROM_GS
#define i_norm vso_norm
in vec3 vso_norm;
#define i_tex vso_tex
in vec2 vso_tex;
#else
#define i_norm gso_norm
in vec3 gso_norm;
#define i_tex gso_tex
in vec2 gso_tex;
#endif
layout(location = 0) out vec4 pso_color;
out float pso_dpth;
struct struct_test
{
};
struct types_test
{
bool a;
int b;
uint c;
float d;
double e;
bvec2 f;
bvec3 g;
bvec4 h;
ivec2 i;
ivec3 j;
ivec4 k;
vec2 l;
vec3 m;
vec3 n;
dvec2 o;
dvec3 p;
dvec4 q;
mat2x2 r0;
mat3x2 r1;
mat4x2 r2;
mat2x3 r3;
mat3x3 r4;
mat4x3 r5;
mat2x4 r6;
mat3x4 r7;
mat4x4 r8;
dmat2x2 s0;
dmat3x2 s1;
dmat4x2 s2;
dmat2x3 s3;
dmat3x3 s4;
dmat4x3 s5;
dmat2x4 s6;
dmat3x4 s7;
dmat4x4 s8;
struct_test some_more_junk;
bool aa[3];
int ba[3][4];
uint ca[3][4][5];
float da[3][4][5][6];
double ea[3][4][5][6][7];
bvec2 fa[3];
bvec3 ga[3];
bvec4 ha[3][4];
ivec2 ia[6][2];
ivec3 ja;
ivec4 ka;
vec2 la;
vec3 ma;
vec3 na;
dvec2 oa;
dvec3 pa;
dvec4 qa;
mat2x2 r0a;
mat3x2 r1a;
mat4x2 r2a;
mat2x3 r3a;
mat3x3 r4a;
mat4x3 r5a;
mat2x4 r6a;
mat3x4 r7a;
mat4x4 r8a;
dmat2x2 s0a;
dmat3x2 s1a;
dmat4x2 s2a;
dmat2x3 s3a;
dmat3x3 s4a;
dmat4x3 s5a;
dmat2x4 s6a;
dmat3x4 s7a;
dmat4x4 s8a;
mat2x2 x;
mat3x3 y;
mat4x4 z;
dmat2x2 x1;
dmat3x3 x2;
dmat4x4 x3;
struct_test some_more_junka[3][6][5];
};
layout(std140) uniform ps_block_0 {
bool no_mor_stuf;
struct_test stufs;
};
layout(std140) uniform ps_block_1 {
float junk[7];
bool something_else[2][2];
};
uniform sampler1D ps_tex_0;
uniform sampler2D ps_tex_1;
uniform sampler3D ps_tex_2;
uniform samplerCube ps_tex_3;
float func2(float x,float y){
float multi = 1.;
return mod(sin(x), cos(y)) * multi;
}
int func0(){
return 89;
}
float func1(int w3,float z,vec4 f,bool r){
if(r)return 0.;
return z * w3 + f.x + f.y * f.z + f.w;
}
void main(){
float z;
int w = 8 + 1;
vec4 f = vec4(1. - 2, 2. * 5, 3. / 6, 4.);
z = 6;
z += 3;
w -= 4;
z++;
w--;
z *= -(4. * w / 2.);
w /= z + 2.;
f = vec3(z, w, w, z);
f.x = f.y + 2. * f.z;
if(z > w){
w++;
z = 4;
}

if(w < z)w++;
if(z == f.x){
z++;
}

else {
float r = 2. * 3. * 4.;
z--;
z *= r;
}
if(f.x != f.y)w += 5;
else z -= 7;
bool b2 = false;
if((af.x == 3) && f.y != 3)f.x--;
else if(bf.x > 4 || (f.y < 4))f.y++;
else if(cf.z == 2 && !b2)f.z = f.y + 2.3;
bool r = f.x + 3. == w - 1.;
while(!r && something_else[z][w - 2]){
while((5 == w) || (w != mix(z, w, f.w))){
f.y = f.x + 2 * f.y;
}
}
for(int i = 0; i < 7; ++i){
f = vec4(i + junk[0], i + 2 * junk[1], i + 3 * junk[2], i + 4 * junk[3]) * junk[2 * i + 1];
}
for(float j = 0.; j != sin(j) && w != z; j += mod(j, w))w = j + z / 2;
z += func1(w, z, f, r);
pso_color = f;
pso_dpth = z;
;
;
}
