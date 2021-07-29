#include "cottoncandy-channel-selector.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CottoncandyChannelSelector");

namespace lorawan {

CottoncandyChannelSelector::CottoncandyChannelSelector(){
    m_threshold = STARTING_THRESHOLD;
    m_interference = 0;
}

void CottoncandyChannelSelector::AddInterference(double rssi){
    if(rssi > SENSITIVITY){
        m_interference += (rssi - SENSITIVITY);
    }
}

bool CottoncandyChannelSelector::SwitchChannel(){
    bool switchChannel = false;

    if(m_interference > m_threshold){
        m_threshold *= MULTIPLIER;
        switchChannel = true;
        printf("Increase threshold: %lf\n",m_threshold);
    }
    else if(m_interference < m_threshold * 0.7 && m_threshold > STARTING_THRESHOLD + DECREMENT){
        m_threshold -= DECREMENT;
        printf("Decrease threshold: %lf\n",m_threshold);
    }

    

    m_interference = 0;
    return switchChannel;
}
}
}