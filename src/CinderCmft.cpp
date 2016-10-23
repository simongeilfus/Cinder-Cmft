#include "CinderCmft.h"
#include "cinder/app/App.h"
#include "cinder/gl/scoped.h"
#include "cmft/clcontext.h"
#include "cmft/print.h"

using namespace ci;
using namespace std;

namespace cmft {

namespace {

	void convertToCubeMap( cmft::Image &image ) 
	{
		if( ! cmft::imageIsCubemap( image ) ) {
			if( cmft::imageIsCubeCross( image ) ) {
				cmft::imageCubemapFromCross( image );
			}
			else if( cmft::imageIsLatLong( image ) ) {
				cmft::imageCubemapFromLatLong( image );
			}
			else if( cmft::imageIsHStrip( image ) )	{
				cmft::imageCubemapFromStrip( image );
			}
			else if( cmft::imageIsVStrip( image ) ) {
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
} // anonymous namespace

cmft::Image surfaceToImage( const ci::Surface &surface )
{
	cmft::Image img;
	cmft::imageCreate( img, surface.getWidth(), surface.getHeight(), 0x000000ff, 1, 1, surface.hasAlpha() ? cmft::TextureFormat::RGBA8 : cmft::TextureFormat::RGB8 );
	memcpy( img.m_data, (void*) surface.getData(), img.m_dataSize );
	return img;
}

ci::gl::TextureCubeMapRef createTextureCubeMap( const cmft::Image &image )
{
	// Input check.
    if( ! imageIsCubemap( image ) ) {
        return gl::TextureCubeMapRef();
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



RadianceFilterOptions& RadianceFilterOptions::enableCaching( bool cache )
{
	mCacheEnabled = cache;
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

IrradianceFilterOptions& IrradianceFilterOptions::enableCaching( bool cache )
{
	mCacheEnabled = cache;
	return *this;
}


ci::gl::TextureCubeMapRef createPmrem( const ci::DataSourceRef &source, uint32_t dstFaceSize, const RadianceFilterOptions &options )
{
	cmft::Image output;

	// if caching is enabled check whether the results have already been calculated
	auto filePath = source->getFilePath();
	auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_pmrem.dds" );
	if( options.mCacheEnabled && fs::exists( cachePath ) ) {
		cmft::imageLoad( output, cachePath.string().c_str(), TextureFormat::RGBA32F );
	}
	// otherwise load the original file and apply the radiance filter
	else {
		// create the input cmft::Image
		auto surface = Surface( loadImage( source ) );
		auto input = surfaceToImage( surface );
		convertToCubeMap( input );
	
		cmft::imageCreate( output, dstFaceSize, dstFaceSize, 0xff0000ff, 7, 6, surface.hasAlpha() ? cmft::TextureFormat::RGBA8 : cmft::TextureFormat::RGB8 );

		// create the opencl context
		//cmft::ClContext clContext;
		
		//int32_t clLoaded = bx::clLoad();
		//if( clLoaded ) {
			//cmft::clPrintDevices();
			//clContext.init( CMFT_CL_VENDOR_ANY_CPU, CMFT_CL_DEVICE_TYPE_CPU, 1 );
		//}
		//else {
			//app::console() << "Problem initializing OpenCL Context." << endl;
		//}

		if( input.m_width != dstFaceSize ) {
			cmft::imageResize( input, dstFaceSize );
		}

		cmft::imageApplyGamma( input, 1.0f / 2.2f );
		
		// apply the filter
	app::console() << "radiance filter" << endl;
		if( ! cmft::imageRadianceFilter( output, dstFaceSize, cmft::LightingModel::BlinnBrdf, options.mExcludeBase, options.mMipCount, options.mGlossScale, options.mGlossBias, input, options.mEdgeFixup, options.mNumCpuProcessingThreads ) ) {//, &clContext ) ) {
			app::console() << "imageRadianceFilter FAILED!" << endl;
		}
		
		cmft::imageApplyGamma( output, 2.2f );
	app::console() << "radiance filter done" << endl;
		// save results if caching is enabled
		if( options.mCacheEnabled ) {
			auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_pmrem" );
			cmft::imageSave( output, cachePath.string().c_str(), ImageFileType::DDS, OutputType::Cubemap, TextureFormat::RGBA16F, true );
		}
		
		// Release OpenCL context and image memory
		//clContext.destroy();
		//if( clLoaded ) {
			//bx::clUnload();
		//}
		cmft::imageUnload( input );
	}
	
	// generate the opengl cubemap texture
	auto outputTex = createTextureCubeMap( output ); 
	
	// Release output image memory
	cmft::imageUnload( output );
	
	return outputTex;
}
ci::gl::TextureCubeMapRef createIem( const ci::DataSourceRef &source, uint32_t dstFaceSize, const IrradianceFilterOptions &options )
{
	cmft::Image output;
	
	// if caching is enabled check whether the results have already been calculated
	auto filePath = source->getFilePath();
	auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_iem.dds" );
	if( options.mCacheEnabled && fs::exists( cachePath ) ) {
		cmft::imageLoad( output, cachePath.string().c_str(), TextureFormat::RGBA32F );
	}
	// otherwise load the original file and apply the radiance filter
	else {

		// create the input and output cmft::Images
		auto surface = Surface( loadImage( source ) );
		auto input = surfaceToImage( surface );
		convertToCubeMap( input );
	
		cmft::imageCreate( output, dstFaceSize, dstFaceSize, 0xff0000ff, 1, 6, cmft::TextureFormat::RGB8 );
	app::console() << "irradiance filter" << endl;
		// apply the filter
		cmft::imageApplyGamma( input, 1.0f / 2.2f );
		cmft::imageIrradianceFilterSh( output, dstFaceSize, input );
		cmft::imageApplyGamma( output, 2.2f );
	app::console() << "irradiance filter done" << endl;	
		// save results if caching is enabled
		if( options.mCacheEnabled ) {
			auto cachePath = filePath.parent_path() / ( filePath.filename().stem().string() + "_iem" );
			cmft::imageSave( output, cachePath.string().c_str(), ImageFileType::DDS, OutputType::Cubemap, TextureFormat::RGBA16F, true );
		}

		// release image memory
		cmft::imageUnload( input );
	}
	
	auto outputTex = createTextureCubeMap( output );

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
		va_end(args2);
		app::console() << string( buf.begin(), buf.end() ) << endl;
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