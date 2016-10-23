#pragma once

#include "cmft/image.h"
#include "cmft/cubemapfilter.h"
#include "cinder/gl/Texture.h"

namespace cmft {

//! Converts a ci::Surface \a surface to a cmft::Image
cmft::Image surfaceToImage( const ci::Surface &surface );
//! Creates a ci::gl::TextureCubeMapRef from a Image \a image
ci::gl::TextureCubeMapRef createTextureCubeMap( const cmft::Image &image );

struct RadianceFilterOptions {
	RadianceFilterOptions() : mCacheEnabled( true ), mLightingModel( LightingModel::BlinnBrdf ), mEdgeFixup( EdgeFixup::None ), mMipCount( 7 ), mGlossScale( 10 ), mGlossBias( 3 ), mNumCpuProcessingThreads( 8 ), mExcludeBase( false ) {}

	//! Sets whether the create functions should check for an existing cache or if it should creates it
	RadianceFilterOptions& enableCaching( bool cache = true );
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
	RadianceFilterOptions& excludeBase( bool exclude );

	bool				mCacheEnabled;
	LightingModel::Enum mLightingModel;
	EdgeFixup::Enum		mEdgeFixup;
	uint8_t				mMipCount; 
	uint8_t				mGlossScale; 
	uint8_t				mGlossBias; 
	uint8_t				mNumCpuProcessingThreads; 
	bool				mExcludeBase;
};

//! Creates a Prefiltered Mipmapped Radiance Environment Map from the surface \a source
ci::gl::TextureCubeMapRef createPmrem( const ci::DataSourceRef &source, uint32_t dstFaceSize, const RadianceFilterOptions &options = RadianceFilterOptions() );

struct IrradianceFilterOptions {
	IrradianceFilterOptions() : mCacheEnabled( true ) {}

	//! Sets whether the create functions should check for an existing cache or if it should creates it
	IrradianceFilterOptions& enableCaching( bool cache = true );

	bool mCacheEnabled;
};

//! Creates a Prefiltered Mipmapped Radiance Environment Map from the surface \a source
ci::gl::TextureCubeMapRef createIem( const ci::DataSourceRef &source, uint32_t dstFaceSize, const IrradianceFilterOptions &options = IrradianceFilterOptions() );

//! Connects cmft messages to cinder console
void connectConsole( bool warning, bool info );
}