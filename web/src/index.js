import LiveDataPage from "./js/LiveDataPage";
import SettingsPage from "./js/SettingsPage";
import "./main.scss";

if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register(`/power/service-worker.js`);
}

window.LiveDataPage = LiveDataPage;
window.SettingsPage = SettingsPage;
