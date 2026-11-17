#include "common.hpp"
#include "RTC/RTP/Codecs/Tools.hpp"
#include <catch2/catch_test_macros.hpp>

SCENARIO(
  "Tools codec predicates allow AV1 simulcast without changing other codecs",
  "[rtp][codecs][tools][av1]")
{
	// Build a mime type from its type and subtype, as the codec parsers do.
	const auto makeMimeType = [](
	                            RTC::RtpCodecMimeType::Type type,
	                            RTC::RtpCodecMimeType::Subtype subtype) -> RTC::RtpCodecMimeType
	{
		RTC::RtpCodecMimeType mimeType;

		mimeType.type    = type;
		mimeType.subtype = subtype;

		return mimeType;
	};

	const auto av1 =
	  makeMimeType(RTC::RtpCodecMimeType::Type::VIDEO, RTC::RtpCodecMimeType::Subtype::AV1);
	const auto vp8 =
	  makeMimeType(RTC::RtpCodecMimeType::Type::VIDEO, RTC::RtpCodecMimeType::Subtype::VP8);
	const auto vp9 =
	  makeMimeType(RTC::RtpCodecMimeType::Type::VIDEO, RTC::RtpCodecMimeType::Subtype::VP9);
	const auto h264 =
	  makeMimeType(RTC::RtpCodecMimeType::Type::VIDEO, RTC::RtpCodecMimeType::Subtype::H264);
	const auto opus =
	  makeMimeType(RTC::RtpCodecMimeType::Type::AUDIO, RTC::RtpCodecMimeType::Subtype::OPUS);

	SECTION("IsValidTypeForCodec() accepts AV1 for SIMULCAST")
	{
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SIMULCAST, av1));
	}

	SECTION("IsValidTypeForCodec() keeps its SIMULCAST values for VP8, VP9 and H264")
	{
		// VP8 and H264 were already valid and stay valid.
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SIMULCAST, vp8));
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SIMULCAST, h264));

		// VP9 was not valid for SIMULCAST and still is not.
		REQUIRE(!RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SIMULCAST, vp9));
	}

	SECTION("IsValidTypeForCodec() keeps SVC, SIMPLE and PIPE behavior")
	{
		// SVC accepted all four video codecs before the change and still does.
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SVC, vp8));
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SVC, vp9));
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SVC, h264));
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SVC, av1));

		// SIMPLE and PIPE accept every codec, including audio.
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::SIMPLE, opus));
		REQUIRE(RTC::RTP::Codecs::Tools::IsValidTypeForCodec(RTC::RtpParameters::Type::PIPE, opus));
	}

	SECTION("CanBeKeyFrame() reports AV1 as key frame capable")
	{
		REQUIRE(RTC::RTP::Codecs::Tools::CanBeKeyFrame(av1));
	}

	SECTION("CanBeKeyFrame() keeps its values for VP8, VP9, H264 and audio")
	{
		REQUIRE(RTC::RTP::Codecs::Tools::CanBeKeyFrame(vp8));
		REQUIRE(RTC::RTP::Codecs::Tools::CanBeKeyFrame(vp9));
		REQUIRE(RTC::RTP::Codecs::Tools::CanBeKeyFrame(h264));

		// Audio never has key frames in this sense.
		REQUIRE(!RTC::RTP::Codecs::Tools::CanBeKeyFrame(opus));
	}
}

SCENARIO("AV1 layered single encoding is still classified as SVC", "[rtp][codecs][tools][av1][svc]")
{
	RTC::RtpParameters rtpParameters;
	RTC::RtpCodecParameters codec;

	codec.mimeType.type    = RTC::RtpCodecMimeType::Type::VIDEO;
	codec.mimeType.subtype = RTC::RtpCodecMimeType::Subtype::AV1;
	codec.payloadType      = 96;
	codec.clockRate        = 90000;

	rtpParameters.codecs.push_back(codec);

	// A single encoding with more than one spatial layer is SVC, not SIMULCAST.
	RTC::RtpEncodingParameters encoding;

	encoding.codecPayloadType = codec.payloadType;
	encoding.spatialLayers    = 2;

	rtpParameters.encodings.push_back(encoding);

	const auto type = RTC::RtpParameters::GetType(rtpParameters);

	REQUIRE(type.has_value());

	// NOLINTNEXTLINE(bugprone-unchecked-optional-access)
	REQUIRE(type.value() == RTC::RtpParameters::Type::SVC);
}
