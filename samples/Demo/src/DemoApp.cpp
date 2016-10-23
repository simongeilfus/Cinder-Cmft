#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "CinderCmft.h"
#include "NvidiaEnablement.h"
#include "Watchdog.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DemoApp : public App {
  public:
	DemoApp();
	void draw() override;

	gl::BatchRef			mTeapot, mSkyBox;
	gl::TextureCubeMapRef	mPmrem, mIem, mEm;
	gl::Texture2dRef		mRoughness, mMetallic, mNormal;
	
	ci::CameraPersp			mCamera;
	ci::CameraUi			mCameraUi;
};

DemoApp::DemoApp()
{
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 50.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );
	
	wd::watch( "*.frag", [this]( const fs::path &path ) { try {
		mTeapot = gl::Batch::create( geom::Sphere().subdivisions( 32 ), gl::GlslProg::create( loadAsset( "teapot.vert" ), loadAsset( "teapot.frag" ) ) );
		mSkyBox = gl::Batch::create( geom::Cube().size( vec3( 100.0f ) ), gl::GlslProg::create( loadAsset( "skybox.vert" ), loadAsset( "skybox.frag" ) ) );
	} catch( gl::GlslProgExc exc ) { app::console() << exc.what() << endl; } } );
	
	// connect cmft output to cinder's console
	cmft::connectConsole( true, true );

	// create the radiance and irradiance environment map
	auto env	= loadAsset( "env_map.jpg" );
	mEm			= gl::TextureCubeMap::create( loadImage( env ) );
	mPmrem		= cmft::createPmrem( env, 256 );
	mIem		= cmft::createIem( env, 64 );

	// load material textures
	auto texFormat = gl::Texture2d::Format().mipmap().minFilter( GL_LINEAR_MIPMAP_LINEAR ).magFilter( GL_LINEAR );
	mRoughness = gl::Texture2d::create( loadImage( loadAsset( "roughness.png" ) ), texFormat );
	mMetallic = gl::Texture2d::create( loadImage( loadAsset( "metallic.png" ) ), texFormat );
	mNormal = gl::Texture2d::create( loadImage( loadAsset( "normal.png" ) ), texFormat );
}

void DemoApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	// set the camera matrices
	gl::setProjectionMatrix( mCamera.getProjectionMatrix() );
	gl::setViewMatrix( mCamera.getViewMatrix() );
	
	// set the gl states and bind the textures
	gl::ScopedDepth scopedDepth( true );
	gl::ScopedState scopedSeamlessCubemap( GL_TEXTURE_CUBE_MAP_SEAMLESS, true );
	gl::ScopedTextureBind scopedTex0( mPmrem, 0 );
	gl::ScopedTextureBind scopedTex1( mIem, 1 );
	gl::ScopedTextureBind scopedTex2( mEm, 2 );
	gl::ScopedTextureBind scopedTex3( mRoughness, 3 );
	gl::ScopedTextureBind scopedTex4( mMetallic, 4 );
	gl::ScopedTextureBind scopedTex5( mNormal, 5 );
	

	// render the teapot
	if( mTeapot ) {
		mTeapot->getGlslProg()->uniform( "uPmremSampler", 0 );
		mTeapot->getGlslProg()->uniform( "uIemSampler", 1 );
		mTeapot->getGlslProg()->uniform( "uRoughnessSampler", 3 );
		mTeapot->getGlslProg()->uniform( "uMetallicSampler", 4 );
		mTeapot->getGlslProg()->uniform( "uNormalSampler", 5 );
		mTeapot->getGlslProg()->uniform( "uCameraPos", mCamera.getEyePoint() );
		mTeapot->draw();
	}
	
	// render the skybox
	if( mSkyBox ) {
		gl::setViewMatrix( mat4( mat3( mCamera.getViewMatrix() ) ) );
		mSkyBox->getGlslProg()->uniform( "uSampler", 2 );
		mSkyBox->draw();
	}
}

CINDER_APP( DemoApp, RendererGl )
/*
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/ConstantConversions.h"
#include "cinder/CameraUi.h"
#include "cmft/cubemapfilter.h"
#include "cmft/clcontext.h"

#include "NvidiaEnablement.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DemoApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	gl::GlslProgRef mGlslProg;
	gl::TextureCubeMapRef mCubemap;
	
	ci::CameraPersp		mCamera;
	ci::CameraUi		mCameraUi;
};

void DemoApp::setup()
{
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 50.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );
	mGlslProg = gl::GlslProg::create( loadAsset( "sky_box.vert" ), loadAsset( "sky_box.frag" ) );
	mCubemap = gl::TextureCubeMap::create( loadImage( loadAsset( "env_map.jpg" ) ) );

	
	console() << glGetString( GL_VENDOR ) << endl;
	cmft::ClContext clContext;
	bool clLoaded = bx::clLoad();
	if( clLoaded ) {
		console() << "clContext: " << clContext.init( CMFT_CL_VENDOR_ANY_GPU, CMFT_CL_DEVICE_TYPE_GPU, 0 ) << endl;
		console() << clContext.m_deviceName << endl;
	}

	auto surf = Surface( loadImage( loadAsset( "env_map.jpg" ) ) );
	cmft::Image img;
	img.m_data = surf.getData();
	img.m_dataSize = surf.getRowBytes() * surf.getHeight();
	img.m_format = cmft::TextureFormat::RGB8;
	img.m_numFaces = 1;
	img.m_numMips = 1;
	img.m_width = surf.getWidth();
	img.m_height = surf.getHeight();



	cmft::Image cubemap;
	cmft::imageCreate( cubemap, 256, 256, 0xff0000ff, 7, 6, cmft::TextureFormat::RGB8 );
	//cmft::imageCopy( cubemap, img );
	cmft::Image imgCubemap;

	if( cmft::imageCubemapFromCross( imgCubemap, img ) ) {
		console() << "Converted" << endl;
		
       // double shCoeffs[SH_COEFF_NUM][3];
		//if( imageShCoeffs( shCoeffs, cubemap ) ) {
		//	console() << "SH Coeffs" << endl;
		//}
		cmft::imageResize( imgCubemap, 256 );
		cmft::imageApplyGamma( imgCubemap, 1.0 / 2.2 );
		//cubemap = imgCubemap;
		//cmft::imageIrradianceFilterSh( cubemap, 64, imgCubemap );
        if( ! cmft::imageRadianceFilter( cubemap, 256, cmft::LightingModel::BlinnBrdf, false, 7, 10.0f, 3.0f, imgCubemap, cmft::EdgeFixup::None, 0, &clContext ) ) {
			console() << "imageRadianceFilter FAILED!" << endl;
		}

		
		//cmft::Image cross;
		//cmft::imageCrossFromCubemap( cross, cubemap, false );

		//Surface outputSurf = Surface( (uint8_t*) cross.m_data, cross.m_width, cross.m_height, cross.m_dataSize / cross.m_height, SurfaceChannelOrder::RGB );
		//console() << "Output to " << ( getAssetPath("") / "test.tga" ) << endl;
		//writeImage( writeFile( getAssetPath("") / "test.jpg" ), outputSurf );
		//cmft::imageSave( cross, ( getAssetPath("") / "test.tga" ).string().c_str(), cmft::ImageFileType::TGA, cmft::OutputType::HCross );
		

		// Input check.
        if( ! imageIsCubemap( cubemap ) ) {
            return;
        }

        // Get destination sizes and offsets.
        uint32_t dstDataSize = 0;
        uint32_t dstMipOffsets[MAX_MIP_NUM];
        const uint8_t bytesPerPixel = cmft::getImageDataInfo( cubemap.m_format ).m_bytesPerPixel;
        for( uint8_t mip = 0; mip < cubemap.m_numMips; ++mip ) {
            dstMipOffsets[mip] = dstDataSize;
            const uint32_t mipSize = glm::max( UINT32_C(1), cubemap.m_width >> mip );
            dstDataSize += mipSize * mipSize * bytesPerPixel;
        }

        // Get source offsets.
        uint32_t cubemapOffsets[CUBE_FACE_NUM][MAX_MIP_NUM];
        cmft::imageGetMipOffsets( cubemapOffsets, cubemap );

		// create opengl texture
		GLenum format = GL_RGB, dataType = GL_UNSIGNED_BYTE;
		auto texFormat = gl::TextureCubeMap::Format();
		switch( cubemap.m_format ) {
		case cmft::TextureFormat::BGR8:
			format = GL_BGR;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_BGR );
			break;
		case cmft::TextureFormat::RGB8:
			format = GL_RGB;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_RGB8 );
			break;
		case cmft::TextureFormat::RGB16:
			format = GL_RGB;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_RGB16 );
			break;
		case cmft::TextureFormat::RGB16F:
			format = GL_RGB;
			dataType = GL_FLOAT;
			texFormat.setInternalFormat( GL_RGB16F );
			break;
		case cmft::TextureFormat::RGB32F:
			format = GL_RGB;
			dataType = GL_FLOAT;
			texFormat.setInternalFormat( GL_RGB32F );
			break;
		case cmft::TextureFormat::BGRA8:
			format = GL_BGRA;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_BGRA );
			break;
		case cmft::TextureFormat::RGBA8:
			format = GL_RGBA;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_RGBA8 );
			break;
		case cmft::TextureFormat::RGBA16:
			format = GL_RGBA;
			dataType = GL_UNSIGNED_BYTE;
			texFormat.setInternalFormat( GL_RGBA16 );
			break;
		case cmft::TextureFormat::RGBA16F:
			format = GL_RGBA;
			dataType = GL_FLOAT;
			texFormat.setInternalFormat( GL_RGBA16F );
			break;
		case cmft::TextureFormat::RGBA32F:
			format = GL_RGBA;
			dataType = GL_FLOAT;
			texFormat.setInternalFormat( GL_RGBA32F );
			break;
		}
		if( cubemap.m_numMips > 1 ) {
			texFormat.enableMipmapping();
			texFormat.setBaseMipmapLevel( 0 );
			texFormat.setMaxMipmapLevel( cubemap.m_numMips );
		}
		console() << "Num Mips: " << (int) cubemap.m_numMips << endl;
		console() << gl::constantToString( texFormat.getInternalFormat() ) << endl;

		gl::TextureData texData;
		texData.allocateDataStore( dstDataSize * 6 );

		texData.setWidth( cubemap.m_width );
		texData.setHeight( cubemap.m_height );
		texData.setInternalFormat( texFormat.getInternalFormat() );
		texData.setDataFormat( format );
		texData.setDepth( 1 );
		texData.setNumFaces( 6 );
		//texData.setUnpackAlignment( 4 );

		mCubemap = gl::TextureCubeMap::create( cubemap.m_width, cubemap.m_height, texFormat );
		//mCubemap->bind();
		gl::ScopedTextureBind scopedTexBind( mCubemap );
		glGenerateMipmap( GL_TEXTURE_CUBE_MAP );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, cubemap.m_numMips );		
            //void* dstData = BX_ALLOC(_allocator, dstDataSize);
            //MALLOC_CHECK(dstData);
		//texData.mapDataStore();
        for( uint8_t mip = 0; mip < cubemap.m_numMips; ++mip ) {
			//texData.push_back( gl::TextureData::Level() );
			//gl::TextureData::Level &tdLevel = texData.back();		
			const uint32_t mipFaceSize = glm::max( UINT32_C(1), cubemap.m_width >> mip);    
			//tdLevel.width = mipFaceSize;
			//tdLevel.height = mipFaceSize;
			//tdLevel.depth = 0;

			for( uint8_t face = 0; face < 6; ++face ) {
				//tdLevel.push_back( gl::TextureData::Face() );
				//gl::TextureData::Face &tdFace = tdLevel.back();

                //const uint8_t* srcFaceData = (const uint8_t*) cubemap.m_data + cubemapOffsets[face][mip];
				GLvoid* pixels = (GLvoid*) ( (uint8_t*) cubemap.m_data + cubemapOffsets[face][mip] );
                //uint8_t* dstFaceData = (uint8_t*) texData.getDataStorePtr( dstMipOffsets[mip] );
				app::console() << mipFaceSize << endl;
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, texFormat.getInternalFormat(), mipFaceSize, mipFaceSize, 0, format, dataType, pixels );
                //const uint32_t mipPitch = mipFaceSize * bytesPerPixel;
				//tdFace.dataSize = mipFaceSize * mipPitch;
				//tdFace.offset = dstMipOffsets[mip];

                for (uint32_t yy = 0; yy < mipFaceSize; ++yy) {
                    //const uint8_t* srcRowData = (const uint8_t*)srcFaceData + yy*mipPitch;
                   // uint8_t* dstRowData = (uint8_t*) dstFaceData + yy*mipPitch;

                   // memcpy(dstRowData, srcRowData, mipPitch);
                }
            }
        }
		//texData.unmapDataStore();
		
		//mCubemap = gl::TextureCubeMap::create( texData, texFormat );
	}

	
	clContext.destroy();
	
	// Unload opencl lib.
	if( clLoaded ) {
		bx::clUnload();
	}
	cmft::imageUnload( cubemap );

}

void DemoApp::mouseDown( MouseEvent event )
{
}

void DemoApp::update()
{
}

void DemoApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	float lod = glm::clamp( (float) ( getMousePos().x - getWindowPos().x ) / (float) getWindowWidth() * 7.0f, 0.0f, 6.0f );
	
	gl::setMatrices( mCamera );

	if( mCubemap ) {
		gl::ScopedState scopedSeamlessCubemap( GL_TEXTURE_CUBE_MAP_SEAMLESS, true );
		gl::ScopedGlslProg scopedGlsl( mGlslProg );
		gl::ScopedTextureBind scopedTex( mCubemap );
		mGlslProg->uniform( "uLod", lod );
		gl::drawCube( vec3(0), vec3( 100.0f ) );
	}

}

CINDER_APP( DemoApp, RendererGl )


*/