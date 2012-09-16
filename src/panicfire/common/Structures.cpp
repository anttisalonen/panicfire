#include <stdexcept>

#include "common/Random.h"

#include "panicfire/common/Structures.h"

namespace PanicFire {

namespace Common {

void MapData::generate(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	data.resize(width * height);
	for(unsigned int j = 0; j < width; j++) {
		for(unsigned int i = 0; i < height; i++) {
			int gl = ::Common::Random::uniform(2, 5);
			int v = ::Common::Random::uniform(0, 3);
			int r = ::Common::Random::uniform(1, 4);
			data[j * height + i].grasslevel = static_cast<GrassLevel>(gl);
			if(v == 0) {
				data[j * height + i].vegetationlevel = static_cast<VegetationLevel>(r);
			}
		}
	}
}

const MapFragment& MapData::getPoint(unsigned int x, unsigned int y) const
{
	if(x >= width || y >= height)
		throw std::runtime_error("MapData: access outside boundary");
	return data[y * height + x];
}

unsigned int MapData::getWidth() const
{
	return width;
}

unsigned int MapData::getHeight() const
{
	return height;
}


}

}

