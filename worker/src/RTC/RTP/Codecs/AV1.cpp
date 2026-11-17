#define MS_CLASS "RTC::RTP::Codecs::AV1"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTP/Codecs/AV1.hpp"
#include "Logger.hpp"
#include "RTC/RTP/Codecs/DependencyDescriptorPayloadDescriptor.hpp"

namespace RTC
{
	namespace RTP
	{
		namespace Codecs
		{
			/* Class methods. */

			void AV1::ProcessRtpPacket(
			  RTP::Packet* packet, Codecs::DependencyDescriptor::Context& dependencyDescriptorContext)
			{
				MS_TRACE();

				auto* const payloadDescriptor =
				  DependencyDescriptorPayloadDescriptor::Parse(packet, dependencyDescriptorContext);

				// Nothing but the Dependency Descriptor tells the layers of an AV1 packet.
				// Without one there are no layers to filter and no key frame to signal, so give
				// the packet the passthrough handler. It forwards the packet unfiltered and
				// reports it as a key frame, so a stream without a Dependency Descriptor cannot
				// gate a simulcast spatial layer switch that would wait for a key frame forever.
				if (!payloadDescriptor)
				{
					packet->SetPayloadDescriptorHandler(new NoDependencyDescriptorHandler());

					return;
				}

				// NOTE: Set as the handler of the codec, so that it also serves a
				// Consumer that did not negotiate the extension. AV1 has no payload
				// descriptor of its own to fall back to.
				//
				// NOTE: That Consumer is sent no Dependency Descriptor either, and an
				// endpoint without one takes every frame it gets for spatial layer 0.
				// Sending it several spatial layers would hence mix them up, so
				// filtering them here is what keeps its video decodable. See
				// https://issues.webrtc.org/issues/560293075.
				packet->SetPayloadDescriptorHandler(
				  new DependencyDescriptorPayloadDescriptorHandler(payloadDescriptor));
			}
		} // namespace Codecs
	} // namespace RTP
} // namespace RTC
