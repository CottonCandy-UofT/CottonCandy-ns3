#ifndef COTTONCANDY_CHANNEL_SELECTOR_H
#define COTTONCANDY_CHANNEL_SELECTOR_H

#include "ns3/object.h"

namespace ns3 {
namespace lorawan {

static const double STARTING_THRESHOLD = 30;
static const double MULTIPLIER = 1.5;
static const double DECREMENT = 10;
static const double SENSITIVITY = -123;

class CottoncandyChannelSelector: public Object{

public:
    CottoncandyChannelSelector();

    void AddInterference(double rssi);

    /**
     * Computes all the values required. Increase/decrease the threshold
     * depending on the accumulated interference margin collected during
     * this session. Reset the interference margin for the next session.
     * Returns true if a new channel needs to be selected.
     */ 
    bool SwitchChannel();

private:
    double m_interference;
    double m_threshold;

};

}
}

#endif