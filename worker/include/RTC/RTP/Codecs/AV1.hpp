#ifndef MS_RTC_RTP_CODECS_AV1_HPP
#define MS_RTC_RTP_CODECS_AV1_HPP

#include "common.hpp"
#include "RTC/RTP/Codecs/DependencyDescriptorPayloadDescriptor.hpp"
#include "RTC/RTP/Codecs/PayloadDescriptorHandler.hpp"
#include "RTC/RTP/Packet.hpp"

namespace RTC
{
	namespace RTP
	{
		namespace Codecs
		{
			class AV1
			{
			private:
				/**
				 * Bits AV1 numbers its frames with. It does not number them itself, so
				 * they are the ones the Dependency Descriptor gives.
				 */
				static constexpr uint8_t FrameIdLength{ DependencyDescriptor::FrameIdLength };

			public:
				static void ProcessRtpPacket(
				  RTP::Packet* packet, Codecs::DependencyDescriptor::Context& dependencyDescriptorContext);

			public:
				/**
				 * Payload descriptor handler for an AV1 packet that carries no Dependency
				 * Descriptor. Such a packet tells neither its layers nor whether it is a key
				 * frame, so it is forwarded unfiltered and reported as a key frame. This way a
				 * stream with no Dependency Descriptor never gates a simulcast spatial layer
				 * switch, which would otherwise wait forever for a key frame it cannot signal.
				 */
				class NoDependencyDescriptorHandler : public Codecs::PayloadDescriptorHandler
				{
				public:
					void Dump(int /* indentation */) const override
					{
					}

					bool Process(
					  Codecs::EncodingContext* /* encodingContext */,
					  RTP::Packet* /* packet */,
					  bool& /* marker */) override
					{
						return true;
					}

					void Encode(RTP::Packet* /* packet */, Codecs::PayloadDescriptor::Encoder* /* encoder */) override
					{
					}

					void Restore(RTP::Packet* /* packet */) override
					{
					}

					uint8_t GetSpatialLayer() const override
					{
						return 0;
					}

					uint8_t GetTemporalLayer() const override
					{
						return 0;
					}

					bool IsKeyFrame() const override
					{
						return true;
					}
				};

			public:
				class EncodingContext : public Codecs::EncodingContext
				{
				public:
					explicit EncodingContext(Codecs::EncodingContext::Params& params)
					  : Codecs::EncodingContext(params, AV1::FrameIdLength)
					{
					}
					~EncodingContext() override = default;
				};
			};
		} // namespace Codecs
	} // namespace RTP
} // namespace RTC

#endif
