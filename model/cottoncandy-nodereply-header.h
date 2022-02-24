/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_NODEREPLY_HEADER_H
#define COTTONCANDY_NODEREPLY_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the extra header of an individual reply packet.
 */
class CottoncandyNodeReplyHeader : public Header
{
public:
    static TypeId GetTypeId (void);
    CottoncandyNodeReplyHeader();
    ~CottoncandyNodeReplyHeader();

    virtual TypeId GetInstanceTypeId(void) const;

    virtual uint32_t GetSerializedSize(void) const;

    virtual void Serialize(Buffer::Iterator start) const;

    virtual uint32_t Deserialize(Buffer::Iterator start);

    virtual void Print (std::ostream &os) const;
    
    virtual void SetOption(uint8_t option);

    virtual uint8_t GetOption() const;

    virtual void SetDataLen(uint8_t len);

    virtual uint8_t GetDataLen() const;

private:
    uint8_t m_option = 0;
    uint8_t m_dataLen; //7-bit only (0-127)
};
}
}

#endif