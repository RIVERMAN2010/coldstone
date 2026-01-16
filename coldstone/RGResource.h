#pragma once
#include <cstdint>
#include <string>

using ResourceIdentifier = uint32_t;

enum class TextureFormat {
	RGBA8,
	RGB8,
	DEPTH24STENCIL8
};
struct TextureDesc {
	uint32_t width;
	uint32_t height;
	TextureFormat format;
	bool generateMipmaps;

	bool operator==(const TextureDesc& other) const {
		return width == other.width &&
			height == other.height &&
			format == other.format &&
			generateMipmaps == other.generateMipmaps;
	}
};

struct BufferDesc {
	uint32_t sizeInBytes;
	bool isDynamic;

	bool operator==(const BufferDesc& other) const {
		return sizeInBytes == other.sizeInBytes &&
			isDynamic == other.isDynamic;
	}
};

struct RGResource {
	std::string name;
	ResourceIdentifier id;
	enum class Type {
		TEXTURE,
		BUFFER
	} type;
	union {
		TextureDesc textureDesc;
		BufferDesc bufferDesc;
	};
};


template<typename T>
class ResourceHandle {
public:
	ResourceIdentifier id = -1;
	bool operator==(const ResourceHandle& other) const {
		return id == other.id;
	}
	bool operator!=(const ResourceHandle& other) const {
		return id != other.id;
	}
	bool isValid() const {
		return id != -1;
	}
};

using TextureHandle = ResourceHandle<struct Texture>;
using BufferHandle = ResourceHandle<struct Buffer>;