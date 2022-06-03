//
// Created by VincentZhang on 6/3/2022.
//

#ifndef HUAHUOENGINEV2_CONTAINERUTILITY_H
#define HUAHUOENGINEV2_CONTAINERUTILITY_H

template<class T> inline
void ContainerClear(T& data)
{
    data.clear();
}

//template<class T, size_t Align> inline
//void ContainerClear(dynamic_array<T, Align>& data)
//{
//    data.clear_dealloc();
//}
//
//template<class T, size_t BlockSize> inline
//void ContainerClear(dynamic_block_array<T, BlockSize>& data)
//{
//    data.clear_dealloc();
//}
//
//template<class TKey, class TValue, class THashFunc, class TKeyEqualFunc> inline
//void ContainerClear(core::hash_map<TKey, TValue, THashFunc, TKeyEqualFunc>& data)
//{
//    data.clear_dealloc();
//}
//
//template<class TValue, class THashFunc, class TKeyEqualFunc> inline
//void ContainerClear(core::hash_set<TValue, THashFunc, TKeyEqualFunc>& data)
//{
//    data.clear_dealloc();
//}
//
//template<class TValue, class TKeyEqualFunc, typename Compare, size_t ALIGN> inline
//void ContainerClear(core::flat_map<TValue, TKeyEqualFunc, Compare, ALIGN>& data)
//{
//    data.clear_dealloc();
//}

#endif //HUAHUOENGINEV2_CONTAINERUTILITY_H
