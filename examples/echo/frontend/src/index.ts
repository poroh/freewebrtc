

async function startRTC() {
    console.log("started");
    const cfg : RTCConfiguration = {};
    const pc = new RTCPeerConnection(cfg);
    const offerOptions : RTCOfferOptions = {
        offerToReceiveAudio: true,
        offerToReceiveVideo: true
    };
    const offer = await pc.createOffer(offerOptions);
    pc.setLocalDescription(offer);
    const offerResp = await fetch('/echo-api/offer', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(offer)
    });
    const answer = await offerResp.json();
    const sdpAnswer: RTCSessionDescriptionInit = {
        type: 'answer',
        sdp: answer.sdp
    };
    pc.setRemoteDescription(sdpAnswer);
    answer.candidates.forEach((c: string) => {
        pc.addIceCandidate({candidate: c, sdpMid: '0'});
    });
}

document.addEventListener('DOMContentLoaded', () =>  {
    const app = document.getElementById('root');
    if (!app) {
        return;
    }
    app.innerHTML = 'Loaded!';
    startRTC();
});




