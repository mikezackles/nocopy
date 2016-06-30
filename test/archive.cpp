//#include <catch.hp>
//
//NOCOPY_FIELD(delta, float);
//NOCOPY_FIELD(first, uint32_t);
//NOCOPY_FIELD(second, uint8_t);
//NOCOPY_ARRAY(third, int8_t, 4);
//NOCOPY_ARRAY(coords, uint8_t, 10);
//NOCOPY_ARRAY(locations, uint32_t, 20);
//
////       field removed in this version
////      field added in this version  |
//template <std::size_t Version> // |  |
//using measurement = //            |  |
//typename //                       |  |
//nocopy::archive< //               |  |
//  Version //                      v  v
//, nocopy::v< delta,               0    >
//, nocopy::v< first,               0, 1 >
//, nocopy::v< second,              0    >
//, nocopy::v< coords,              0    >
//, nocopy::v< locations,           1    >
//>::datapack;
