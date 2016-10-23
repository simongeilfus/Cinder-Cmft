# Cinder-Cmft

Cubemap Filtering Tools for Cinder based on [cmft](https://github.com/dariomanesku/cmft).  

See [cmft repository](https://github.com/dariomanesku/cmft) for more information.

The block is roughly organised around two types of functions; Convenient functions to load, filter and cache environment maps as `cinder::gl::TextureCubeMapRef`: 

```c++
// create the skybox, radiance and irradiance environment map
// this will by default cache env_pmrem.dds and env_iem.dds for
// a much faster initialization on the next run
gl::TextureCubeMapRef	mPmrem, mIem, mEm;

auto imgPath	= getAssetPath( "env.hdr" );
mEm				= cmft::createTextureCubemap( imgPath );
mPmrem			= cmft::createPmrem( imgPath, 256 );
mIem			= cmft::createIem( imgPath, 64 );
```

And conversion and helpers functions to interface directly between `cmft::Images` and `cinder::Surfaces` and `cinder::gl::TextureCubeMaps` :

```c++
// load an image with cinder and convert it to a cmft::Image
auto surface = Surface( loadImage( loadAsset( "env_cross.exr" ) ) );
auto input = cmft::surfaceToImage( surface );

// prepare the output cmft::Image
cmft::Image output;
cmft::imageCreate( output, dstFaceSize, dstFaceSize, 0xff0000ff, 1, 6, cmft::TextureFormat::RGB32F );

// apply an sh irradiance filter
cmft::imageApplyGamma( input, 2.2f );
cmft::imageIrradianceFilterSh( output, dstFaceSize, input );
cmft::imageApplyGamma( output, 1.0f / 2.2f );

// convert the cmft::Image to a gl::TextureCubeMap
gl::TextureCubeMapRef cubemap = cmft::createTextureCubemap( output );

// release cmft image memory
cmft::imageUnload( input );
cmft::imageUnload( output );
```

Screenshots from the demo app (material made with Substance Designer and HDR Envmaps from [NoEmotionHDRs](http://noemotionhdrs.net)) :

![Image](/res/demo_screenshots.jpg)