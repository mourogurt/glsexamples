#version 330 core

//Cairo tiling by nimitz (stormoid.com) (twitter: @stormoid)

//Inspired by Petri Leskinen's "Cairo Pentagonal Tiling" http://pixelero.wordpress.com/page/2/

/*
        Cairo pentagonal tiling made using a "voronoi-like" function,
        the main difference is that half the tiles are reflected and the centers are
        displaced symmetrically.

        Please let me know if you can think of a simple way to get all the edges.
*/

uniform block
{
    vec2 iResolution;
    float iGlobalTime;
    mat4 proj;
    mat4 view;
};


float hash( float n ){ return fract(sin(n)*43758.5453);}


vec3 field(const in vec2 p)
{
    vec2 fp = fract(p);
    vec2 ip = floor(p);
    vec3 rz = vec3(1.);
    float of = sin(iGlobalTime*0.5*0.6)*.5;
    float rf = mod(ip.x+ip.y,2.0);
    fp.x = rf-fp.x*sign(rf-.5);
    for(float j=0.; j<=1.; j++)
        for(float i=0.; i<=1.; i++)
        {
            vec2 b = vec2(j, i);
            float sgn = sign(j-0.5);
            float cmp = float(j == i);
            vec2 o = vec2(sgn*cmp,-sgn*(1.-cmp));
            vec2 sp = fp - b + of*o;
            b += o;
            float d = dot(sp,sp);
            if( d<rz.x )
            {
                rz.z = rz.x;
                rz.x = d;
                b.x = rf-b.x*sign(rf-.5);
                rz.y = hash( dot(ip+b,vec2(7.0,113.0) ) );
            }
            else if( d<rz.z )
                rz.z = d;
        }
    float d = dot(fp-.5,fp-.5);
    d += 0.4;
    if (d < rz.x)
    {
        rz.z = rz.x;
        rz.x = d;
        rz.y = hash( dot(ip,vec2(7.0,113.0) ) );
    }
    else if(d < rz.z )
        rz.z = d;
    rz.z = rz.z-rz.x;
    return rz;
}


void main(void)
{
    vec2 p = gl_FragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;
    float a = sin(iGlobalTime*0.5)*.5;
    float b = .5;
    p *= 6.+sin(iGlobalTime*0.5*0.4);
    p.x += iGlobalTime*0.5;
    p.y += sin(iGlobalTime*0.5)*0.5;
    vec3 rz = field(p);
    vec3 col = (sin(vec3(.2,.55,.8)+rz.y*4.+2.)+0.4)*0.6+0.5;
    col *= 1.-rz.x;
    col *= smoothstep(0.,.04,rz.z);
    gl_FragColor = vec4(col,1.0);
}
