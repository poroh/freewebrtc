

async function startRTC() {
    console.log("started");
    const cfg : RTCConfiguration = {};
    const pc = new RTCPeerConnection(cfg);
    const offerOptions : RTCOfferOptions = {
        offerToReceiveAudio: true,
        offerToReceiveVideo: true
    };
    const offer = await pc.createOffer(offerOptions);
    if (offer.sdp) {
        const answer = await fetch('/echo-api/offer', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(offer)
        });
        console.log(answer);
    }
}

document.addEventListener('DOMContentLoaded', function ()  {
    console.log(arguments);
    const app = document.getElementById('root');
    if (!app) {
        return;
    }
    app.innerHTML = 'Loaded!';
    startRTC();
});




