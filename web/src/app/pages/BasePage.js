import notifyUpgradableMixin from "../mixins/NotifyUpgradableMixin";
import nativeEventsMixin from "../mixins/NativeEventsMixin";

export default class BasePage {
    loading = true;

    init(html) {
        Object.assign(this, notifyUpgradableMixin);
        Object.assign(this, nativeEventsMixin);

        document.getElementById('app').innerHTML = html;

        this.checkForUpdate();
    }

    destroy() {
        this.removeAllListeners();
        document.getElementById('app').innerHTML = '';
    }

    redirectTo(page) {
        window.location.hash = page;
    }

    showContent() {
        if (this.loading) {
            this.loading = false;
            document.getElementById('content').style.display = 'block';
            document.getElementById('app-spinner').style.display = 'none';
        }
    }

    showLoading() {
        if (!this.loading) {
            this.loading = true;
            document.getElementById('content').style.display = 'none';
            document.getElementById('app-spinner').style.display = 'block';
        }
    }
}
