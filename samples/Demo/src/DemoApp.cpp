#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "CinderCmft.h"
//#include "NvidiaEnablement.h"
#include "Watchdog.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DemoApp : public App {
  public:
	DemoApp();
	void draw() override;
	void keyDown( KeyEvent event ) override;

	gl::BatchRef			mTeapot, mSkyBox;
	gl::TextureCubeMapRef	mPmrem, mIem, mEm;
	gl::Texture2dRef		mRoughness, mMetallic, mNormal, mBaseColor;
	
	ci::CameraPersp			mCamera;
	ci::CameraUi			mCameraUi;
};

DemoApp::DemoApp()
{
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 50.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );
	
	wd::watch( "*.frag", [this]( const fs::path &path ) { try {
		mTeapot = gl::Batch::create( geom::Sphere().subdivisions( 64 ), gl::GlslProg::create( loadAsset( "teapot.vert" ), loadAsset( "teapot.frag" ) ) );
		mSkyBox = gl::Batch::create( geom::Cube().size( vec3( 100.0f ) ), gl::GlslProg::create( loadAsset( "skybox.vert" ), loadAsset( "skybox.frag" ) ) );
	} catch( gl::GlslProgExc exc ) { app::console() << exc.what() << endl; } } );
	
	// connect cmft output to cinder's console
	cmft::connectConsole( true, true );

	// create the radiance and irradiance environment map
	string imgPath	= getAssetPath( "cave_entry_in_the_forest.hdr" ).string();
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

void DemoApp::keyDown( KeyEvent event ) 
{
	if( event.getCode() == KeyEvent::KEY_1 ) {
		string imgPath	= getAssetPath( "bonifacio_street.hdr" ).string();
		mEm				= cmft::createTextureCubemap( imgPath );
		mPmrem			= cmft::createPmrem( imgPath, 256 );
		mIem			= cmft::createIem( imgPath, 64 );
	}
	else if( event.getCode() == KeyEvent::KEY_2 ) {
		string imgPath	= getAssetPath( "cave_entry_in_the_forest.hdr" ).string();
		mEm				= cmft::createTextureCubemap( imgPath );
		mPmrem			= cmft::createPmrem( imgPath, 256 );
		mIem			= cmft::createIem( imgPath, 64 );
	}
	else if( event.getCode() == KeyEvent::KEY_3 ) {
		string imgPath	= getAssetPath( "corsica_beach.hdr" ).string();
		mEm				= cmft::createTextureCubemap( imgPath );
		mPmrem			= cmft::createPmrem( imgPath, 256 );
		mIem			= cmft::createIem( imgPath, 64 );
	}
	else if( event.getCode() == KeyEvent::KEY_4 ) {
		string imgPath	= getAssetPath( "Soft_1Front.hdr" ).string();
		mEm				= cmft::createTextureCubemap( imgPath );
		mPmrem			= cmft::createPmrem( imgPath, 256 );
		mIem			= cmft::createIem( imgPath, 64 );
	}
	else if( event.getCode() == KeyEvent::KEY_5 ) {
		string imgPath	= getAssetPath( "studio_03.hdr" ).string();
		mEm				= cmft::createTextureCubemap( imgPath );
		mPmrem			= cmft::createPmrem( imgPath, 256 );
		mIem			= cmft::createIem( imgPath, 64 );
	}
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
	gl::ScopedTextureBind scopedTex6( mBaseColor, 6 );
	

	// render the teapot
	if( mTeapot ) {
		mTeapot->getGlslProg()->uniform( "uPmremSampler", 0 );
		mTeapot->getGlslProg()->uniform( "uIemSampler", 1 );
		mTeapot->getGlslProg()->uniform( "uRoughnessSampler", 3 );
		mTeapot->getGlslProg()->uniform( "uMetallicSampler", 4 );
		mTeapot->getGlslProg()->uniform( "uNormalSampler", 5 );
		mTeapot->getGlslProg()->uniform( "uBaseColorSampler", 6 );
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

CINDER_APP( DemoApp, RendererGl( RendererGl::Options().msaa( 8 ) ) )