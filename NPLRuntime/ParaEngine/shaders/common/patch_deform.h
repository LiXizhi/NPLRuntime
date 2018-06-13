    // declare a 4x4 grid of 
    // world heights and 
    // surface normals
    struct PatchPoints
    {
      float4 normh[16] ;
    };
    const PatchPoints pc : patchCorners;

    // a function to deform vertices
    // using the patch height values
    void PatchDeform(
        in out float3 Pos,
        uniform float4x4 mWorld)
    {
      // note:
      // we assume the source model is
      // scaled to fit within a 3x3 unit
      // box.

      // Find the x,y integer position
      // and cell-based scalars
      int x;
      int y;
      float sx = modf(Pos.x, x);
      float sy = modf(Pos.y, y);

      // compute the index into our 4x4
      // array of values and read the
      // four corner values of the cell
      // we are in
      int index = (y*4)+x;
      float4 z0 = pc.normh[index];
      float4 z1 = pc.normh[index+1];
      float4 z2 = pc.normh[index+4];
      float4 z3 = pc.normh[index+5];

      // interpolate between the
      // four corners using sx and sy
      float4 zl = lerp(z0,z1,sx); 
      float4 zh = lerp(z2,z3,sx); 
      float4 zi = lerp(zl,zh,sy); 
  
      // convert our point to world
      // space and offset by the
      // interpolated height in zi
      Pos = mul(float4(Pos, 1), mWorld);
      Pos.z = Pos.z+zi.w;
    }

    // a function to deform vertices
    // using the patch height values.
    // Normals are aligned with the
    // underlying surface normals
    // of the ground plane.
    void PatchDeformWithNormals(
        in out float3 Pos,
        out float3 Norm,
        uniform float4x4 mWorld)
    {
      // note:
      // we assume the source model is
      // scaled to fit within a 3x3 unit
      // box.

      // Find the x,y integer position
      // and cell-based scalars
      int x;
      int y;
      float sx = modf(Pos.x, x);
      float sy = modf(Pos.y, y);

      // compute the index into our 4x4
      // array of values and read the
      // four corner values of the cell
      // we are in
      int index = (y*4)+x;
      float4 z0 = pc.normh[index];
      float4 z1 = pc.normh[index+1];
      float4 z2 = pc.normh[index+4];
      float4 z3 = pc.normh[index+5];

      // interpolate between the
      // four corners using sx and sy
      float4 zl = lerp(z0,z1,sx); 
      float4 zh = lerp(z2,z3,sx); 
      float4 zi = lerp(zl,zh,sy); 
  
      // return the interpolated ground normal
      Norm = normalize(zi.rgb);

      // convert our point to world
      // space and offset by the
      // interpolated height in zi
      Pos = mul(float4(Pos, 1), mWorld);
      Pos.z = Pos.z+zi.w;
    }
