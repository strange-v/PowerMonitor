export default class BasePage {
    loading = true;

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
