struct VS_OUTPUT
{
    float4 Pos : POSITION;
};

VS_OUTPUT SimpleTransform(float4  iPos : POSITION)
{
    VS_OUTPUT  o;

    o.Pos       = iPos;
    
    return o;
}

///////////////////////////////////////////////////////////
// Technique ////////////////////////////////////////
///////////////////////////////////////////////////////////
technique t0
{
    pass p0
    {        
        VertexShader = compile vs_1_1 SimpleTransform();
    }
}
