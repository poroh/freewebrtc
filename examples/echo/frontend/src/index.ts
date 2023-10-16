

async function startRTC() {
    console.log("started");
    const cfg : RTCConfiguration = {};
    const pc = new RTCPeerConnection(cfg);
    const offerOptions : RTCOfferOptions = {
        offerToReceiveAudio: true,
        offerToReceiveVideo: true
    };
    const offer = await pc.createOffer(offerOptions);
    pc.onicecandidate = async (event) => {
        console.log(event);
        if (!event.candidate) {
            const answer = await fetch('/echo-api/offer', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(pc.localDescription)
            });
            console.log(answer);
        }
    };
    pc.setLocalDescription(offer);
}

document.addEventListener('DOMContentLoaded', () =>  {
    const app = document.getElementById('root');
    if (!app) {
        return;
    }
    app.innerHTML = 'Loaded!';
    startRTC();
});




