import uPlot from 'uplot';
import Formatter from 'app/utils/Formatter';
import { POWER, VOLTAGE } from '../ChartType'

let overlay;

export function tooltipPlugin(type) {
    let over, bound, bLeft, bTop, overlayHeight = 20;

    function syncBounds() {
        let bbox = over.getBoundingClientRect();
        bLeft = bbox.left;
        bTop = bbox.top;
    }

    if (overlay) {
        overlay.remove();
        overlay = null;
    }

    overlay = document.createElement("div");
    overlay.id = "overlay";
    overlay.style.display = "none";
    overlay.style.position = "absolute";
    overlay.style.right = "30px";
    overlay.style.top = "8px";
    overlay.style.color = '#bdbdbd';
    document.getElementById('chart-container').appendChild(overlay);
    
    return {
        hooks: {
            init: u => {
                over = u.over;
                bound = over;
                over.onmouseenter = () => {
                    overlay.style.display = "block";
                };
                over.onmouseleave = () => {
                    overlay.style.display = "none";
                };
            },
            setSize: u => {
                syncBounds();
            },
            setCursor: u => {
                const { top, idx } = u.cursor;
                
                const fmtValue = type === POWER ? Formatter.power : Formatter.voltage;
                const fmtDate = uPlot.fmtDate("{HH}:{mm}"); 
                const x = fmtDate(new Date(u.data[0][idx] * 1e3));
                const min = fmtValue(u.data[1][idx]);
                const max = fmtValue(u.data[3][idx]);

                overlay.innerHTML = `From <b>${min}</b> to <b>${max}</b> at <b>${x}</b>`;
            }
        }
    };
}