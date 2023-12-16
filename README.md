# directx61

<img src="https://github.com/kurlyak/directx61/blob/main/pics/cube3dc.png" alt="3d development directx 6" width=600 />

<img src="https://github.com/kurlyak/directx61/blob/main/pics/cube3dt.png" alt="3d development directx 6" width=600 />

These examples for DirectX 6.1 were developed on WinAPI Visual Stduio 2005, using the DirectX 6.1 SDK.

001-Text_Tri_D3D3

Example for Visual Studio 2005 WinAPI, DirectX 6.0 Direct3D3 Device, we create a texture from a BMP image with 24-bit color depth, and using GetDC() and BitBlt() we copy the BMP image into our texture, which we overlay on the triangle. In this example, we create a full-fledged pointer to the texture LPDIRECT3DTEXTURE2, in contrast to the fact that Direct3D2 instead of a pointer to the texture we get the so-called Texture Handler. Example without using an index buffer.



002-Textured_Cube_D3D3

Example for Visual Studio 2005 WinAPI. An example similar to the previous one, instead of a triangle on the screen there is a rotating cube. The cube rotates on the screen around the Y axis. An example of rendering using an index buffer. Create a texture for the cube using GetDC() and BitBlt(). Create a texture from a BMP image with 24 bit color depth.



003-Textured_Cube_SoftRend_D3D3

Example for Visual Studio 2005 WinAPI. The same as the previous example, only the vertices are multiplied by matrices, this is a software rendering project, there is a function for multiplying the vertices of a cube by the matrices of the world, view, projection. Drawing the screen coordinates of the cube (triangles) is assigned to DirectX 6.0. An example of rendering using an index buffer. Create a texture for the cube using GetDC() and BitBlt(). Create a texture from a BMP image with 24 bit color depth. This programming method (software calculation of model vertices, drawing triangles using DirectX 6.0) was used in the computer game Tomb Raider 3, which was created in 1998.



004-Textured_Cube_ZBuff_D3D3

Example for Visual Studio 2005 WinAPI. Same as the previous example, only a Z buffer is added.



005-Textured_Cube_ZBuff_LockTex_D3D3

Example for Visual Studio 2005 WinAPI. The same as the previous example, only the texture image is created differently - the texture image is copied to the surface using the Lock() function.



006-Textured_Cube_ZBuff_LockTex8bit_D3D3

The same as the previous one, only the texture is loaded from a BMP image with a color depth of 8 bits.



007-Color_Cube_D3D3

An example similar to the previous ones, Direct3D3. The difference is that the application creates a colored cube, not a textured cube.



008-Textured_Cube_TexHandle_D3D2

Now we are using Direct3D2. Create a texture and get its Texture Handler. Then we install this Texutre Handler before displaying the cube. In the app, a textured cube rotates on the screen. Create a texture from a BMP image with 24 bit color depth.




009-Textured_Cube_TexHandle_ZBuff_D3D2

Same as the previous example (we switched to Direct3D2). Addition - creating a Z buffer for the application. Create a texture from a BMP image with 24 bit color depth.