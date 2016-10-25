#include "CinderCmft.h"
#include "cinder/app/App.h"
#include "cinder/gl/scoped.h"
#include "cmft/clcontext.h"
#include "cmft/print.h"

using namespace ci;
using namespace std;

namespace cmft {

void surfaceToImage( const ci::Surface &surface, cmft::Image &output )
{	
	if( cmft::imageIsValid( output ) ) {
		cmft::imageUnload( output );
	}
	cmft::imageCreate( output, surface.getWidth(), surface.getHeight(), 0x000000ff, 1, 1, surface.hasAlpha() ? cmft::TextureFormat::RGBA8 : cmft::TextureFormat::RGB8 );
	memcpy( output.m_data, (void*) surface.getData(), output.m_dataSize );
}
void textureCubemapToImage( const ci::gl::TextureCubeMapRef &cubemap, cmft::Image &output )
{
	GLenum format, dataType;
	cmft::TextureFormat::Enum texFormat = cmft::TextureFormat::RGBA32F;
	switch( cubemap->getInternalFormat() ) {
	case GL_BGR:
		format = GL_BGR;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::BGR8;
		break;
	case GL_RGB8:
		format = GL_RGB;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::RGB8;
		break;
	case GL_RGB16:
		format = GL_RGB;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::RGB16;
		break;
	case GL_RGB16F:
		format = GL_RGB;
		dataType = GL_HALF_FLOAT;
		// forcing alpha channel. see https://github.com/dariomanesku/cmft/issues/21
		texFormat = TextureFormat::RGBA16F;
			// cmft::TextureFormat::RGB16F;
		break;
	case GL_RGB32F:
		format = GL_RGB;
		dataType = GL_FLOAT;
		texFormat = cmft::TextureFormat::RGB32F;
		break;
	case GL_BGRA:
		format = GL_BGRA;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::BGRA8;
		break;
	case GL_RGBA8:
		format = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::RGBA8;
		break;
	case GL_RGBA16:
		format = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		texFormat = cmft::TextureFormat::RGBA16;
		break;
	case GL_RGBA16F:
		format = GL_RGBA;
		dataType = GL_HALF_FLOAT;
		texFormat = cmft::TextureFormat::RGBA16F;
		break;
	case GL_RGBA32F:
		format = GL_RGBA;
		dataType = GL_FLOAT;
		texFormat = cmft::TextureFormat::RGBA32F;
		break;
	}

	cmft::Image faceList[6];
	gl::ScopedTextureBind scopedTex( cubemap );
	for( int face = 0 ; face < 6; ++face ) {
		cmft::imageCreate( faceList[face], cubemap->getWidth(), cubemap->getHeight(), 0x0, 1, 1, texFormat );
		glGetTexImage( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, format, dataType, faceList[face].m_data );
	}
	
	cmft::imageCubemapFromFaceList( output, faceList );
}

void convertToCubemap( cmft::Image &image ) 
{
	if( ! cmft::imageIsCubemap( image ) ) {
		if( cmft::imageIsCubeCross( image ) ) {
			cmft::imageCubemapFromCross( image );
		}
		else if( cmft::imageIsLatLong( image ) ) {
			cmft::imageCubemapFromLatLong( image );
		}
		else if( cmft::imageIsHStrip( image ) || cmft::imageIsVStrip( image ) )	{
			cmft::imageCubemapFromStrip( image );
		}
		else if( cmft::imageIsOctant( image ) )	{
			cmft::imageCubemapFromOctant( image );
		}
		else if( ! cmft::imageCubemapFromCross( image ) ) {
			app::console() << "problem converting!!!!" << endl;
		}
	}
}

ci::gl::TextureCubeMapRef createTextureCubemap( cmft::Image &image )
{
	// Input check.
    if( ! imageIsCubemap( image ) ) {
        convertToCubemap( image );
    }

    // Get destination sizes and offsets.
    uint32_t dstDataSize = 0;
    uint32_t dstMipOffsets[MAX_MIP_NUM];
    const uint8_t bytesPerPixel = cmft::getImageDataInfo( image.m_format ).m_bytesPerPixel;
    for( uint8_t mip = 0; mip < image.m_numMips; ++mip ) {
        dstMipOffsets[mip] = dstDataSize;
        const uint32_t mipSize = glm::max( UINT32_C(1), image.m_width >> mip );
        dstDataSize += mipSize * mipSize * bytesPerPixel;
    }

    // Get source offsets.
    uint32_t cubemapOffsets[CUBE_FACE_NUM][MAX_MIP_NUM];
    cmft::imageGetMipOffsets( cubemapOffsets, image );

	// create opengl texture
	GLenum format = GL_RGB, dataType = GL_UNSIGNED_BYTE;
	auto texFormat = gl::TextureCubeMap::Format();
	switch( image.m_format ) {
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
		dataType = GL_HALF_FLOAT;
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
		dataType = GL_HALF_FLOAT;
		texFormat.setInternalFormat( GL_RGBA16F );
		break;
	case cmft::TextureFormat::RGBA32F:
		format = GL_RGBA;
		dataType = GL_FLOAT;
		texFormat.setInternalFormat( GL_RGBA32F );
		break;
	}
	if( image.m_numMips > 1 ) {
		texFormat.enableMipmapping();
		texFormat.setBaseMipmapLevel( 0 );
		texFormat.setMaxMipmapLevel( image.m_numMips );
	}
	
	auto cubemap = gl::TextureCubeMap::create( image.m_width, image.m_height, texFormat );
	gl::ScopedTextureBind scopedTexBind( cubemap );
	glGenerateMipmap( GL_TEXTURE_CUBE_MAP );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, image.m_numMips );		
       
	for( uint8_t face = 0; face < 6; ++face ) {
		for( uint8_t mip = 0; mip < image.m_numMips; ++mip ) {
			const uint32_t mipFaceSize = glm::max( UINT32_C(1), image.m_width >> mip);    
			GLvoid* pixels = (GLvoid*) ( (uint8_t*) image.m_data + cubemapOffsets[face][mip] );
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, texFormat.getInternalFormat(), mipFaceSize, mipFaceSize, 0, format, dataType, pixels );
		}
    }

	return cubemap;
}

ci::gl::TextureCubeMapRef createTextureCubemap( const ci::fs::path &filePath )
{
	cmft::Image input;
	bool imageLoaded = cmft::imageLoad( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F )
					|| cmft::imageLoadStb( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F );
	
	if( ! imageLoaded ) {
		app::console() << "Problem loading Image" << endl;
	}

	return createTextureCubemap( input );
}


RadianceFilterOptions& RadianceFilterOptions::gammaCorrection( float gammaInput, float gammaOutput )
{
	mGammaInput = gammaInput;
	mGammaOutput = gammaOutput;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::lightingModel( LightingModel::Enum model )
{
	mLightingModel = model;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::edgeFixup( EdgeFixup::Enum fixup )
{
	mEdgeFixup = fixup;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::mipCount( uint8_t mips )
{
	mMipCount = mips;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::glossScale( uint8_t scale )
{
	mGlossScale = scale;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::glossBias( uint8_t bias )
{
	mGlossBias = bias;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::numCpuProcessingThreads( uint8_t numThreads )
{
	mNumCpuProcessingThreads = numThreads;
	return *this;
}
RadianceFilterOptions& RadianceFilterOptions::excludeBase( bool exclude )
{
	mExcludeBase = exclude;
	return *this;
}

namespace {
	void loadCl()
	{
		static bool loaded = false;
		if( ! loaded ) {
			bx::clLoad();
			cmft::clPrintDevices();
			app::App::get()->getSignalCleanup().connect( []() {
				bx::clUnload();
			} );
			loaded = true;
		}
	}
}

bool createPmrem( cmft::Image &input, cmft::Image &output, uint32_t dstFaceSize, const RadianceFilterOptions &options )
{
	// prepare input / output
	if( ! cmft::imageIsCubemap( input ) ) {
		convertToCubemap( input );
	}
	cmft::imageCreate( output, dstFaceSize, dstFaceSize, 0xff0000ff, 7, 6, cmft::TextureFormat::RGBA32F );

	// create the opencl context
	auto clContext = make_unique<cmft::ClContext>();
	loadCl();
	clContext->init( CMFT_CL_VENDOR_ANY_GPU, CMFT_CL_DEVICE_TYPE_GPU );

	if( input.m_width != dstFaceSize ) {
		cmft::imageResize( input, dstFaceSize );
	}

	// apply the filter
	cmft::imageApplyGamma( input, options.mGammaInput );
	cmft::imageRadianceFilter( output, dstFaceSize, cmft::LightingModel::BlinnBrdf, options.mExcludeBase, options.mMipCount, options.mGlossScale, options.mGlossBias, input, options.mEdgeFixup, options.mNumCpuProcessingThreads, clContext.get() );
	cmft::imageApplyGamma( output, options.mGammaOutput );
		
	// Release OpenCL context and image memory
	clContext->destroy();
	clContext.reset();

	return true;//filtered;
}
gl::TextureCubeMapRef createPmrem( cmft::Image &input, uint32_t dstFaceSize, const RadianceFilterOptions &options )
{
	// prepare and generate output
	cmft::Image output;	
	createPmrem( input, output, dstFaceSize, options );

	// generate the opengl cubemap texture
	auto outputTex = createTextureCubemap( output );

	// release image memory
	cmft::imageUnload( output );

	return outputTex;
}

ci::gl::TextureCubeMapRef createPmrem( const ci::Surface &source, uint32_t dstFaceSize, const RadianceFilterOptions &options )
{
	cmft::Image input;
	surfaceToImage( source, input );
	
	// generate the opengl cubemap texture
	auto outputTex = createPmrem( input, dstFaceSize, options );
	
	// Release output image memory
	cmft::imageUnload( input );

	return outputTex;
}
ci::gl::TextureCubeMapRef createPmrem( const ci::fs::path &filePath, uint32_t dstFaceSize, const RadianceFilterOptions &options, bool cacheEnabled )
{
	cmft::Image output;

	// if caching is enabled check whether the results have already been calculated
	auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_pmrem.dds" );
	if( cacheEnabled && fs::exists( cachePath ) && 
		( cmft::imageLoad( output, cachePath.string().c_str(), cmft::TextureFormat::RGBA32F )
		|| cmft::imageLoadStb( output, cachePath.string().c_str(), cmft::TextureFormat::RGBA32F ) ) ) {
	}
	// otherwise load the original file and apply the radiance filter
	else {
		cmft::Image input;
		bool imageLoaded = cmft::imageLoad( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F )
						|| cmft::imageLoadStb( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F );
	
		if( ! imageLoaded ) {
			app::console() << "Problem loading Image" << endl;
		}
		
		createPmrem( input, output, dstFaceSize, options );
		cmft::imageUnload( input );
		
		// save results if caching is enabled
		if( cacheEnabled ) {
			auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_pmrem" );
			cmft::imageSave( output, cachePath.string().c_str(), ImageFileType::DDS, OutputType::Cubemap, TextureFormat::RGBA16F, true );
		}
	}
	
	// generate the opengl cubemap texture
	auto outputTex = createTextureCubemap( output ); 
	
	// Release output image memory
	cmft::imageUnload( output );
	
	return outputTex;
}

IrradianceFilterOptions& IrradianceFilterOptions::gammaCorrection( float gammaInput, float gammaOutput )
{
	mGammaInput = gammaInput;
	mGammaOutput = gammaOutput;
	return *this;
}

bool createIem( cmft::Image &input, cmft::Image &output, uint32_t dstFaceSize, const IrradianceFilterOptions &options )
{	
	// prepare input / output
	if( ! cmft::imageIsCubemap( input ) ) {
		convertToCubemap( input );
	}
	cmft::imageCreate( output, dstFaceSize, dstFaceSize, 0xff0000ff, 1, 6, cmft::TextureFormat::RGB32F );

	// apply the filter
	cmft::imageApplyGamma( input, options.mGammaInput );
	bool filtered = cmft::imageIrradianceFilterSh( output, dstFaceSize, input );
	cmft::imageApplyGamma( output, options.mGammaOutput );

	return filtered;
}
ci::gl::TextureCubeMapRef createIem( cmft::Image &input, uint32_t dstFaceSize, const IrradianceFilterOptions &options )
{	
	// generate output
	cmft::Image output;
	createIem( input, output, dstFaceSize, options );

	// generate the opengl cubemap texture
	auto outputTex = createTextureCubemap( output );

	// release image memory
	cmft::imageUnload( output );

	return outputTex;
}

ci::gl::TextureCubeMapRef createIem( const ci::Surface &source, uint32_t dstFaceSize, const IrradianceFilterOptions &options )
{
	cmft::Image input;
	surfaceToImage( source, input );

	// generate the opengl cubemap texture
	auto outputTex = createIem( input, dstFaceSize, options );

	// release image memory
	cmft::imageUnload( input );
	
	return outputTex;
}

ci::gl::TextureCubeMapRef createIem( const ci::fs::path &filePath, uint32_t dstFaceSize, const IrradianceFilterOptions &options, bool cacheEnabled )
{
	cmft::Image output;
	
	// if caching is enabled check whether the results have already been calculated
	auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_iem.dds" );
	if( cacheEnabled && fs::exists( cachePath ) && 
		( cmft::imageLoad( output, cachePath.string().c_str(), cmft::TextureFormat::RGBA32F )
		|| cmft::imageLoadStb( output, cachePath.string().c_str(), cmft::TextureFormat::RGBA32F ) ) ) {
	}
	// otherwise load the original file and apply the radiance filter
	else {
		cmft::Image input;
		bool imageLoaded = cmft::imageLoad( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F )
						|| cmft::imageLoadStb( input, filePath.string().c_str(), cmft::TextureFormat::RGBA32F );
	
		if( ! imageLoaded ) {
			app::console() << "Problem loading Image" << endl;
		}

		createIem( input, output, dstFaceSize, options );

		// save results if caching is enabled
		if( cacheEnabled ) {
			auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_iem" );
			cmft::imageSave( output, cachePath.string().c_str(), ImageFileType::DDS, OutputType::Cubemap, TextureFormat::RGBA16F, true );
		}

		// release image memory
		cmft::imageUnload( input );
	}
	
	auto outputTex = createTextureCubemap( output );

	// release image memory
	cmft::imageUnload( output );
	
	return outputTex;
}

namespace {
	int uglyCPrintFunc( const char* format, ... ) {
		va_list args;
		va_start(args, format);
		va_list args2;
		va_copy(args2, args);
		std::vector<char> buf( 1 + std::vsnprintf( nullptr, 0, format, args ) );
		va_end( args );
		std::vsnprintf( buf.data(), buf.size(), format, args2 );
		string output = string( buf.begin(), buf.end() );
		app::console() << output << endl;
		va_end(args2);
		return 1;
	}
} // anonymous namespace

void connectConsole( bool warning, bool info )
{
	if( warning ) {
		cmft::setWarningPrintf( &uglyCPrintFunc );
	}
	
	if( info ) {
		cmft::setInfoPrintf( &uglyCPrintFunc );	
	}
}

}