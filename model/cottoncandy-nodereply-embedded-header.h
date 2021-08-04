/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_NODEREPLY_EMBEDDED_HEADER_H
#define COTTONCANDY_NODEREPLY_EMBEDDED_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the extra header of an embedded reply inside an aggregated node reply.
 */
class CottoncandyNodeReplyEmbeddedHeader : public Header
{
public:
    static TypeId GetTypeId (void);
    CottoncandyNodeReplyEmbeddedHeader();
    ~CottoncandyNodeReplyEmbeddedHeader();

    virtual TypeId GetInstanceTypeId(void) const;

    virtual uint32_t GetSerializedSize(void) const;

    virtual void Serialize(Buffer::Iterator start) const;

    virtual uint32_t Deserialize(Buffer::Iterator start);

    virtual void Print (std::ostream &os) const;

    virtual void SetSrc(uint16_t srcAddr);

    virtual uint16_t GetSrc() const;

    virtual void SetDataLen(uint8_t len);

    virtual uint8_t GetDataLen() const;

private:
    uint16_t m_srcAddr;
    uint8_t m_dataLen; 
};
}
}

#endif