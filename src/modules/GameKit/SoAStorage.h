//
// Created by eva00 on 25-5-20.
//
#pragma once
#include <vector>
#include <optional>
#include <cstdint>
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>
#include <type_traits>

using UID = std::uint64_t;

// SoAStorage是一个可以支持快速添加、快速删除和快速遍历的多态存储容器.
// SoAStorageは、迅速な追加、削除、および多態的な走査をサポートするストレージコンテナです。
template<typename T>
class SoAStorage {
    struct Slot {
        std::unique_ptr<T> data = nullptr; // 数据指针
        UID uid = 0; // 当前槽位绑定的 UID
        bool active = false;
    };

    std::vector<Slot> slots; // 数据槽位
    std::unordered_map<UID, size_t> uidToIndex; // UID → 槽位索引
    std::vector<size_t> freeIndices; // 空闲槽位索引池
    UID nextUID = 1; // UID 单调递增，永不复用

public:
    // 添加节点 并返回其唯一标识符
    // コンポーネントを追加し、その一意の識別子を返します
    UID Add(std::unique_ptr<T> data) {
        size_t index;
        if (!freeIndices.empty()) {
            index = freeIndices.back();
            freeIndices.pop_back();
        } else {
            index = slots.size();
            slots.emplace_back();
        }

        UID uid = nextUID++;
        slots[index].data = std::move(data);
        slots[index].uid = uid;
        slots[index].active = true;
        uidToIndex[uid] = index;

        return uid;
    }

    // 删除节点
    // コンポーネントを削除します
    void Remove(UID uid) {
        auto it = uidToIndex.find(uid);
        if (it == uidToIndex.end()) {
            return;
        }

        size_t index = it->second;
        if (slots[index].uid != uid) {
            return;
        }

        slots[index].data.reset();
        slots[index].data = nullptr;
        slots[index].active = false;
        slots[index].uid = 0;
        uidToIndex.erase(it);
        freeIndices.push_back(index);
    }

    // 检查节点是否存在
    // コンポーネントが存在するかどうかを確認します
    bool Has(UID uid) const {
        auto it = uidToIndex.find(uid);
        if (it == uidToIndex.end()) {
            return false;
        }

        size_t index = it->second;
        return slots[index].active && slots[index].uid == uid;
    }

    // 获取节点的引用
    // コンポーネントを取得します
    T *Get(UID uid) {
        auto it = uidToIndex.find(uid);
        if (it == uidToIndex.end()) {
            return nullptr;
        }
        size_t index = it->second;
        if (!slots[index].active || slots[index].uid != uid) {
            return nullptr;
        }
        return slots[index].data.get();
    }

    // 获取节点的常量引用
    // コンポーネントの定数参照を取得します
    const T *Get(UID uid) const {
        auto it = uidToIndex.find(uid);
        if (it == uidToIndex.end()) {
            return nullptr;
        }
        size_t index = it->second;
        if (!slots[index].active || slots[index].uid != uid) {
            return nullptr;
        }
        return slots[index].data.get();
    }

    // 遍历节点（模板方式，更适合调试器）
    // アクティブなコンポーネントを走査します
    template<typename Func>
    void ForEachActive(Func &&fn) {
        for (uint32_t i = 0; i < slots.size(); ++i) {
            if (!slots[i].active || !slots[i].data) {
                continue;
            }
            assert(slots[i].data.get() != reinterpret_cast<T*>(0xdddddddddddddddd)); // MSVC标志值
            fn(slots[i].uid, slots[i].data.get());
        }
    }

    // 清空存储 不释放内存空间
    // ストレージをクリアしますが、メモリは解放しません
    void Clear() {
        for (auto &slot: slots) {
            slot.data.reset();
            slot.data = nullptr;
            slot.active = false;
            slot.uid = 0;
        }
        uidToIndex.clear();
        freeIndices.clear();
    }

    // 清空存储 并释放内存空间
    // ストレージをクリアし、メモリを解放します
    void Reset() {
        slots.clear();
        uidToIndex.clear();
        freeIndices.clear();
        nextUID = 1;
    }
};
