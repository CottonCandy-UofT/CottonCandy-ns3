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

    virtual void SetSeqNum(uint8_t seqNum);

    virtual uint8_t GetSeqNum() const;

    virtual void SetDataLen(uint8_t len);

    virtual uint8_t GetDataLen() const;

    virtual void SetAggregated(bool aggregated);

    virtual bool GetAggregated() const;

private:
    uint8_t m_seqNum;
    uint8_t m_dataLen; //7-bit only (0-127)
    bool m_aggregated = false;
};
}
}

#endif