#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/ObjLoader.h"

#include "CinderImGui.h"
#include "CinderCmft.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CustomEnvApp : public App {
  public:
	CustomEnvApp();
	void update() override;
	void draw() override;

	void createEnvironment();
	void updateEnvironmentMaps();

	gl::BatchRef			mModel, mSkyBox, mCornelBox;
	gl::TextureCubeMapRef	mPmrem, mIem, mEm;
	gl::FboCubeMapRef		mEmFbo;
	gl::Texture2dRef		mCornelBoxLightMap;
	
	ci::CameraPersp			mCamera;
	ci::CameraUi			mCameraUi;

	float					mExposure, mWhiteLevel, mRoughness, mMetallic;
	
	bool					mUseCornelBox;
	Color					mCeilingLightColor, mFrontLightColor;
	float					mCornelBoxExposure, mCeilingLightStrength, mCeilingLightWidth, mFrontLightStrength, mFrontLightWidth;
};

CustomEnvApp::CustomEnvApp()
: mExposure( 1.0f ),
mWhiteLevel( 0.6f ),
mRoughness( 0.035f ), 
mMetallic( 0.0f ),
mCeilingLightColor( Color::white() ),
mCeilingLightStrength( 2.0f ),
mCeilingLightWidth( 0.6f ),	
mFrontLightColor( Color( 1.0f, 0.3f, 0.26f ) ),
mFrontLightStrength( 1.0f ),
mFrontLightWidth( 2.0f ),
mUseCornelBox( false ),
mCornelBoxExposure( 0.65f )
{
	// setup ui
	ui::initialize();

	// setup camera and camera ui
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 50.0f, 0.1f, 1000.0f );
	mCamera.lookAt( vec3( 3.0f ), vec3( 0.0f ) );
	mCameraUi = CameraUi( &mCamera, getWindow() );
	
	// load cornel box model and light map
	ObjLoader objLoader( loadAsset( "CornellBox.obj" ) );
	mCornelBoxLightMap = gl::Texture2d::create( loadImage( loadAsset( "CornelBox.hdr" ) ) );

	// setup batches
	try {
		mModel = gl::Batch::create( geom::Sphere().subdivisions( 64 ), gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) ) );
		mCornelBox = gl::Batch::create( objLoader >> geom::Translate( vec3( 0.0f, -0.5f, 0.0f ) ), gl::GlslProg::create( loadAsset( "shader_cornelbox.vert" ), loadAsset( "shader_cornelbox.frag" ) ) );
		mSkyBox = gl::Batch::create( geom::Cube().size( vec3( 100.0f ) ), gl::GlslProg::create( loadAsset( "skybox.vert" ), loadAsset( "skybox.frag" ) ) );
	} 
	catch( gl::GlslProgExc exc ) { 
		app::console() << exc.what() << endl; 
	}
	
	// connect cmft output to cinder's console
	cmft::connectConsole( true, true );

	// setup env fbo
	auto fboFormat = gl::FboCubeMap::Format().textureCubeMapFormat( gl::TextureCubeMap::Format().internalFormat( GL_RGBA32F ) );
	mEmFbo = gl::FboCubeMap::create( 1024, 1024, fboFormat );
	createEnvironment();
	updateEnvironmentMaps();
}

void CustomEnvApp::createEnvironment()
{
	// render environment to the em cubemap
	gl::ScopedFramebuffer scopedFbo( mEmFbo );
	gl::ScopedViewport scopedViewport( ivec2( 0, 0 ), mEmFbo->getSize() );
	gl::ScopedMatrices scopedMatrices;
	gl::setProjectionMatrix( ci::CameraPersp( mEmFbo->getWidth(), mEmFbo->getHeight(), 90.0f, 0.1f, 100.0f ).getProjectionMatrix() );
	for( uint8_t dir = 0; dir < 6; ++dir ) {
		gl::setViewMatrix( mEmFbo->calcViewMatrix( GL_TEXTURE_CUBE_MAP_POSITIVE_X + dir, vec3( 0 ) ) );
		mEmFbo->bindFramebufferFace( GL_TEXTURE_CUBE_MAP_POSITIVE_X + dir );
		
		gl::clear( ColorA::black() );

		if( ! mUseCornelBox ) {
			gl::ScopedColor scopedColor0( mCeilingLightColor * mCeilingLightStrength );
			gl::drawCube( vec3( -mCeilingLightWidth * 2.0f , 3, 0 ),	vec3( mCeilingLightWidth, 0.1f, mCeilingLightWidth * 3.0f ) );
			gl::drawCube( vec3( 0, 3, 0 ),								vec3( mCeilingLightWidth, 0.1f, mCeilingLightWidth * 3.0f ) );
			gl::drawCube( vec3( mCeilingLightWidth * 2.0f, 3,0 ),		vec3( mCeilingLightWidth, 0.1f, mCeilingLightWidth * 3.0f ) );
		
			gl::ScopedColor scopedColor1( mFrontLightColor * mFrontLightStrength );
			gl::drawCube( vec3( 0, 0, 3 ), vec3( mFrontLightWidth, 3, 0.1f ) );
		}
		else {
			gl::ScopedTextureBind scopedTexBind( mCornelBoxLightMap );
			mCornelBox->getGlslProg()->uniform( "uApplyGammaCorrection", false );
			mCornelBox->getGlslProg()->uniform( "uExposure", mExposure * mCornelBoxExposure );
			mCornelBox->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
			mCornelBox->draw();
		}
	}
	mEm = mEmFbo->getTextureCubeMap();
	mPmrem = mEmFbo->getTextureCubeMap();
	mIem = mEmFbo->getTextureCubeMap();
}
void CustomEnvApp::updateEnvironmentMaps()
{
	cmft::Image input;
	cmft::textureCubemapToImage( mEm, input );
	mPmrem = cmft::createPmrem( input, 256, cmft::RadianceFilterOptions().gammaCorrection( 1, 1 ) );
	mIem = cmft::createIem( input, 64, cmft::IrradianceFilterOptions().gammaCorrection( 1,1 ) );
	cmft::imageUnload( input );
}


void CustomEnvApp::update()
{
	// user interface
	{
		ui::ScopedWindow scopedWindow( "Options" );
		if( ui::CollapsingHeader( "Runtime Options", "", true, true ) ) {
			ui::DragFloat( "Roughness", &mRoughness, 0.01f, 0.0f, 1.0f );
			ui::DragFloat( "Metallic", &mMetallic, 0.01f, 0.0f, 1.0f );
			ui::DragFloat( "Exposure", &mExposure, 0.01f, 0.001f, 20.0f );
			ui::DragFloat( "White Level", &mWhiteLevel, 0.01f, 0.001f, 20.0f );
		}

		if( ui::CollapsingHeader( "Environment Options", "", true, true ) ) {
			bool envChanged = false;
			envChanged |= ui::Checkbox( "UseCornelBox", &mUseCornelBox );

			if( mUseCornelBox ) {
				envChanged |= ui::DragFloat( "CornelBox Exposure", &mCornelBoxExposure, 0.01f, 0.0f, 10.0f );
			}
			else {
				envChanged |= ui::ColorEdit3( "CeilingLight Color", &mCeilingLightColor[0] );
				envChanged |= ui::DragFloat( "CeilingLight Strength", &mCeilingLightStrength, 0.01f, 0.0f, 10.0f );
				envChanged |= ui::DragFloat( "CeilingLight Width", &mCeilingLightWidth, 0.01f, 0.0f, 10.0f );
				envChanged |= ui::ColorEdit3( "FrontLight Color", &mFrontLightColor[0] );
				envChanged |= ui::DragFloat( "FrontLight Strength", &mFrontLightStrength, 0.01f, 0.0f, 10.0f );
				envChanged |= ui::DragFloat( "FrontLight Width", &mFrontLightWidth, 0.01f, 0.0f, 10.0f );
			}

			// render the env map
			if( envChanged ) {
				createEnvironment();
			}
		}

		// re-generate the radiance and irradiance maps
		if( ui::Button( "Update Maps" ) ) {
			updateEnvironmentMaps();
		}
	}
}

void CustomEnvApp::draw()
{
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

	// render the test model
	if( mModel ) {
		mModel->getGlslProg()->uniform( "uPmremSampler", 0 );
		mModel->getGlslProg()->uniform( "uIemSampler", 1 );
		mModel->getGlslProg()->uniform( "uExposure", mExposure );
		mModel->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
		mModel->getGlslProg()->uniform( "uRoughness", mRoughness );
		mModel->getGlslProg()->uniform( "uMetallic", mMetallic );
		mModel->getGlslProg()->uniform( "uIsCornelBox", mUseCornelBox );
		mModel->getGlslProg()->uniform( "uCameraPos", mCamera.getEyePoint() );
		mModel->draw();
	}
	
	// render the environment
	if( ! mUseCornelBox && mSkyBox ) {
		gl::setViewMatrix( mat4( mat3( mCamera.getViewMatrix() ) ) );
		mSkyBox->getGlslProg()->uniform( "uSampler", 2 );
		mSkyBox->getGlslProg()->uniform( "uExposure", mExposure );
		mSkyBox->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
		mSkyBox->draw();
	}
	else if( mCornelBox ) {
		gl::ScopedTextureBind scopedTexBind( mCornelBoxLightMap );
		gl::ScopedModelMatrix scopedModel;
		gl::setModelMatrix( glm::scale( vec3( 4.0f ) ) );
		mCornelBox->getGlslProg()->uniform( "uExposure", mExposure );
		mCornelBox->getGlslProg()->uniform( "uWhiteLevel", mWhiteLevel );
		mCornelBox->getGlslProg()->uniform( "uApplyGammaCorrection", true );
		mCornelBox->draw();
	}
}

CINDER_APP( CustomEnvApp, RendererGl( RendererGl::Options().msaa( 8 ) ) )
