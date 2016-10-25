#pragma once

#include "cmft/image.h"
#include "cmft/cubemapfilter.h"
#include "cinder/gl/Texture.h"

namespace cmft {

//! Converts a ci::Surface \a surface to a cmft::Image
void surfaceToImage( const ci::Surface &surface, cmft::Image &output );
//! Converts a ci::gl::TextureCubeMap \a surface to a cmft::Image
void textureCubemapToImage( const ci::gl::TextureCubeMapRef &cubemap, cmft::Image &output );
//! Converts a cmft::Image \a image to a Cubemap cmft::Image
void convertToCubemap( cmft::Image &image );

//! Creates a ci::gl::TextureCubeMapRef from a Image \a image
ci::gl::TextureCubeMapRef	createTextureCubemap( cmft::Image &image );
//! Creates a ci::gl::TextureCubeMapRef from an image at \a filePath
ci::gl::TextureCubeMapRef	createTextureCubemap( const ci::fs::path &filePath );

struct RadianceFilterOptions {
	RadianceFilterOptions() : mLightingModel( LightingModel::BlinnBrdf ), mEdgeFixup( EdgeFixup::None ), mMipCount( 7 ), mGlossScale( 10 ), mGlossBias( 3 ), mNumCpuProcessingThreads( 0 ), mExcludeBase( false ), mGammaInput( 1.0f ), mGammaOutput( 1.0f ) {}

	//! Sets the gamma correction applied to the input and output of the radiance filter
	RadianceFilterOptions& gammaCorrection( float gammaInput, float gammaOutput );
	//! Sets the type of lighting model used by the radiance filter
	RadianceFilterOptions& lightingModel( LightingModel::Enum model );
	//! Sets the filter has to apply a warp edge fixup. 
	RadianceFilterOptions& edgeFixup( EdgeFixup::Enum fixup );
	//! Sets the desired number of mipmap
	RadianceFilterOptions& mipCount( uint8_t mips ); 
	//! Sets the gloss scale used by the radiance filter
	RadianceFilterOptions& glossScale( uint8_t scale ); 
	//! Sets the gloss bias used by the radiance filter
	RadianceFilterOptions& glossBias( uint8_t bias ); 
	//! Sets the number of cpu processing threads used by the radiance filter
	RadianceFilterOptions& numCpuProcessingThreads( uint8_t numThreads ); 
	//! Sets excludeBase filter param?
	RadianceFilterOptions& excludeBase( bool exclude );

	bool				mExcludeBase;
	LightingModel::Enum mLightingModel;
	EdgeFixup::Enum		mEdgeFixup;
	float				mGammaInput, mGammaOutput;
	uint8_t				mMipCount, mGlossScale, mGlossBias, mNumCpuProcessingThreads; 
};

//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cmft::Image \a input to a cmft::Image \a output
bool						createPmrem( cmft::Image &input, cmft::Image &output, uint32_t dstFaceSize, const RadianceFilterOptions &options = RadianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cmft::Image \a input
ci::gl::TextureCubeMapRef	createPmrem( cmft::Image &input, uint32_t dstFaceSize, const RadianceFilterOptions &options = RadianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a ci::Surface \a source
ci::gl::TextureCubeMapRef	createPmrem( const ci::Surface &source, uint32_t dstFaceSize, const RadianceFilterOptions &options = RadianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cubemap image at \a filePath
ci::gl::TextureCubeMapRef	createPmrem( const ci::fs::path &filePath, uint32_t dstFaceSize, const RadianceFilterOptions &options = RadianceFilterOptions(), bool cacheEnabled = true );


struct IrradianceFilterOptions {
	IrradianceFilterOptions() : mGammaInput( 1.0f ), mGammaOutput( 1.0f ) {}

	//! Sets the gamma correction applied to the input and output of the radiance filter
	IrradianceFilterOptions& gammaCorrection( float gammaInput, float gammaOutput );

	float				mGammaInput, mGammaOutput;
};

//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cmft::Image \a input to cmft::Image \a output
bool						createIem( cmft::Image &input, cmft::Image &output, uint32_t dstFaceSize, const IrradianceFilterOptions &options = IrradianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cmft::Image \a input
ci::gl::TextureCubeMapRef	createIem( cmft::Image &input, uint32_t dstFaceSize, const IrradianceFilterOptions &options = IrradianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a ci::Surface \a source
ci::gl::TextureCubeMapRef	createIem( const ci::Surface &source, uint32_t dstFaceSize, const IrradianceFilterOptions &options = IrradianceFilterOptions() );
//! Creates a Prefiltered Mipmapped Radiance Environment Map from a cubemap image at \a filePath
ci::gl::TextureCubeMapRef	createIem( const ci::fs::path &filePath, uint32_t dstFaceSize, const IrradianceFilterOptions &options = IrradianceFilterOptions(), bool cacheEnabled = true );

//! Connects cmft messages to cinder console
void connectConsole( bool warning, bool info );

}