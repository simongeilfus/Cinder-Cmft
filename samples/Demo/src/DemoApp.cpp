#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "CinderImGui.h"
#include "CinderCmft.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DemoApp : public App {
  public:
	DemoApp();
	void draw() override;

	gl::BatchRef			mModel, mSkyBox;
	gl::TextureCubeMapRef	mPmrem, mIem, mEm;
	gl::Texture2dRef		mRoughness, mMetallic, mNormal, mBaseColor;
	
	ci::CameraPersp			mCamera;
	ci::CameraUi			mCameraUi;

	int						mCurrentEnv;
	float					mExposure, mWhiteLevel;
	bool					mShowOriginal;
};

DemoApp::DemoApp()
: mCurrentEnv( 0 ),
mExposure( 1.0f ),
mWhiteLevel( 0.6f ),
mShowOriginal( true )
{
	// setup ui
	ui::initialize();

	// setup camera and camera ui
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 50.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );
	
	try {
		mModel = gl::Batch::create( geom::Sphere().subdivisions( 64 ), gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) ) );
		mSkyBox = gl::Batch::create( geom::Cube().size( vec3( 100.0f ) ), gl::GlslProg::create( loadAsset( "skybox.vert" ), loadAsset( "skybox.frag" ) ) );
	} 
	catch( gl::GlslProgExc exc ) 
	{ 
		app::console() << exc.what() << endl; 
	}
	
	// connect cmft output to cinder's console
	cmft::connectConsole( true, true );

	// create the skybox, radiance and irradiance environment map
	// this will by default cache env_pmrem.dds and env_iem.dds for
	// a much faster initialization on the next run
	auto imgPath	= getAssetPath( "04-12_Sun_A.hdr" );
	mEm				= cmft::createTextureCubemap( imgPath );
	mPmrem			= cmft::createPmrem( imgPath, 256 );
	mIem			= cmft::createIem( imgPath, 64 );
	

	// load material textures
	auto texFormat	= gl::Texture2d::Format().mipmap().minFilter( GL_LINEAR_MIPMAP_LINEAR ).magFilter( GL_LINEAR );
	mBaseColor		= gl::Texture2d::create( loadImage( loadAsset( "basecolor.png" ) ), texFormat );
	mRoughness		= gl::Texture2d::create( loadImage( loadAsset( "roughness.png" ) ), texFormat );
	mMetallic		= gl::Texture2d::create( loadImage( loadAsset( "metallic.png" ) ), texFormat );
	mNormal			= gl::Texture2d::create( loadImage( loadAsset( "normal.png" ) ), texFormat );
}

void DemoApp::draw()
{
	// user interface
	{
		ui::ScopedWindow scopedWindow( "Options" );
		ui::Checkbox( "Show Original", &mShowOriginal );
		static vector<string> hdrs = { "04-12_Sun_A.hdr", "02-04_Garage.hdr", "08-21_Swiss_B.hdr", "05-20_Park_B.hdr", "08-07_Night_B.hdr", "08-08_Sunset_B.hdr" };
		if( ui::Combo( "Environment", &mCurrentEnv, hdrs ) ) {
			auto imgPath	= getAssetPath( hdrs[mCurrentEnv] );
			mEm				= cmft::createTextureCubemap( imgPath );
			mPmrem			= cmft::createPmrem( imgPath, 256 );
			mIem			= cmft::createIem( imgPath, 64 );
		}
		ui::DragFloat( "Exposure", &mExposure, 0.01f, 0.001f, 20.0f );
		ui::DragFloat( "White Level", &mWhiteLevel, 0.01f, 0.001f, 20.0f );
	}

	// clear buffers
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
	gl::ScopedTextureBind scopedTex6( mBaseColor, 6 );
	

	// render the test model
	if( mModel ) {
		mModel->getGlslProg()->uniform( "uPmremSampler", 0 );
		mModel->getGlslProg()->uniform( "uIemSampler", 1 );
		mModel->getGlslProg()->uniform( "uRoughnessSampler", 3 );
		mModel->getGlslProg()->uniform( "uMetallicSampler", 4 );
		mModel->getGlslProg()->uniform( "uNormalSampler", 5 );
		mModel->getGlslProg()->uniform( "uBaseColorSampler", 6 );
		mModel->getGlslProg()->uniform( "uExposure", mExposure );
		mModel->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
		mModel->getGlslProg()->uniform( "uCameraPos", mCamera.getEyePoint() );
		mModel->draw();
	}
	
	// render the skybox
	if( mSkyBox ) {
		gl::setViewMatrix( mat4( mat3( mCamera.getViewMatrix() ) ) );
		mSkyBox->getGlslProg()->uniform( "uSampler", mShowOriginal ? 2 : 1 );
		mSkyBox->getGlslProg()->uniform( "uExposure", mExposure );
		mSkyBox->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
		mSkyBox->draw();
	}
}

CINDER_APP( DemoApp, RendererGl( RendererGl::Options().msaa( 8 ) ) )