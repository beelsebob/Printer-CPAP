#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "Utilities/MsTime.hpp"

namespace pcp {
    template <typename T>
    struct ESCOperation {
        using DataPoint = std::pair<MsTime, T>;

        ESCOperation(const std::string& name, const std::initializer_list<DataPoint>& speeds) : _name(name), _speeds(speeds) {}

        T at(MsTime time, MsTime& outTimeToNextUpdate) const;

        bool empty(void) const { return _speeds.empty(); }

        std::string _name;
        std::vector<DataPoint> _speeds;
    };

    template <typename T>
    T ESCOperation<T>::at(MsTime time, MsTime& outTimeToNextUpdate) const {
        const auto& secondLast = *(_speeds.end() - 2);
        const auto& last = *(_speeds.end() - 1);

        MsTime preTime = secondLast.first;
        MsTime postTime = last.first;
        T preValue = secondLast.second;
        T postValue = last.second;

        for (auto iter = _speeds.begin(); iter != _speeds.end(); ++iter) {
            if (iter->first > time) {
                if (iter == _speeds.begin()) {
                    preTime = iter->first;
                    postTime = iter->first;
                    preValue = iter->second;
                    postValue = iter->second;
                } else {
                    preTime = (iter - 1)->first;
                    preValue = (iter - 1)->second;
                    postTime = iter->first;
                    postValue = iter->second;
                }
                break;
            }
        }

        T value = preValue;
        MsTime timeToNextUpdate = 20;
        if (preValue == postValue) {
            if (postTime > time) {
                timeToNextUpdate = postTime - time;
            }
        } else {
            const MsTime timePeriod = postTime - preTime;
            const MsTime timeIn = clamp(time - preTime, 0_ms, timePeriod);
            const T valueIncrease = postValue - preValue;
            const T scaledValueIncrease = timePeriod == 0 ? 0 : (timeIn * valueIncrease) / timePeriod;
            value = preValue + scaledValueIncrease;
        }

        return value;
    }
}  // namespace pcp
