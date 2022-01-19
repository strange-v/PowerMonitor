import BasePage from "app/pages/BasePage";
import html from './template.html';
import scss from './style.scss';
import { POWER, VOLTAGE, getTypeLable } from './ChartType'
import Menu from 'app/components/menu/Menu';
import Formatter from 'app/utils/Formatter';
import { tooltipPlugin } from './plugins/Tooltip';
import { touchZoomPlugin } from './plugins/TouchZoom';
import uPlot from 'uplot';
import uPlotcss from 'uplot/dist/uPlot.min.css';

export default class ChartPage extends BasePage {
    init(params) {
        super.init(html);

        this._type = params.type;
        this._initControls();
        this._loadData();
    }

    destroy() {
        this._menu.destroy();
        document.body.style.overscrollBehavior = 'auto';
    }

    _loadData(e) {
        this._data = [];
        fetch(`/power/api/chart?type=${this._type}`)
            .then(response => response.json())
            .then(data => this._transformData(data))
            .then(data => this._data = data)
            .then(_ => this.showContent())
            .then(_ => this._buildChart())
            .catch(e => console.log(e));
    }

    _onResize() {
        this._buildChart();
    }

    _buildChart() {
        const container = document.getElementById('chart-container'),
            fmtDate = uPlot.fmtDate("{HH}:{mm}");

        let opts = {
            class: "my-chart",
            width: document.body.clientWidth,
            height: document.body.clientHeight - 50,
            plugins: [
                tooltipPlugin(this._type),
                touchZoomPlugin(),
            ],
            series: [
                {},
                {
                    spanGaps: true,
                    width: 1,
                    points: { show: false },
                    stroke: "rgba(126, 87 , 194, 0.3)",
                },
                {
                    spanGaps: true,
                    stroke: "#7E57C2",
                    width: 1,
                    points: { show: false },
                    dash: [10, 10],
                },
                {
                    spanGaps: true,
                    width: 1,
                    points: { show: false },
                    stroke: "rgba(126, 87 , 194, 0.3)",
                }
            ],
            bands: [
                {
                    series: [3, 2],
                    fill: "rgba(126, 87 , 194, 0.3)",
                },
                {
                    series: [2, 1],
                    fill: "rgba(126, 87 , 194, 0.3)",
                },
            ],
            axes: [
                {
                    stroke: '#bdbdbd',
                    values: (self, ticks) => ticks.map(v => fmtDate(new Date(v * 1e3))),
                    gap: 0,
                    size: 25,
                    grid: {
                        show: true,
                        width: 1,
                        stroke: "rgba(255,255,255,0.1)",
                    },
                    border: {
                        show: true,
                        stroke: "rgba(255,255,255,0.5)"
                    }
                },
                {
                    space: 50,
                    gap: 0,
                    stroke: '#bdbdbd',
                    grid: {
                        show: true,
                        width: 1,
                        stroke: "rgba(255,255,255,0.1)",
                    },
                    values: (self, ticks) => ticks.map(v => Formatter.number(v, 1)),
                },
            ],
            cursor: {
                y: false,
            },
            legend: {
                show: false
            }
        };

        if (this._chart)
            this._chart.destroy();

        this._chart = new uPlot(opts, this._data, container);
    }

    _transformData(data) {
        const minValues = [],
            avgValues = [],
            maxValues = [],
            labels = [];
        let day, hours, minutes;

        data.forEach(d => {
            minutes = d[0] & 0xFF;
            hours = d[0] >> 8 & 0xFF;
            day = d[0] >> 16;

            const now = new Date;
            now.setUTCDate(day);
            now.setUTCHours(hours);
            now.setUTCMinutes(minutes);

            labels.push(now.getTime() / 1000 - 30);
            minValues.push(d[1]);
            avgValues.push((d[1] + d[2])/2);
            maxValues.push(d[2]);
        });

        return [ labels, minValues, avgValues, maxValues ];
    }

    _onTypeChange(e, type) {
        e.stopPropagation();

        const btnMenu = document.getElementById('value-type');
        btnMenu.getElementsByClassName('text')[0].innerHTML = getTypeLable(type);
        this._type = type;

        this._loadData();
    }

    _initControls() {
        document.body.style.overscrollBehavior = 'contain';

        this.addListener(window, 'resize', this._onResize);
        this.addListener(document.getElementById('reload'), 'click', this._loadData);

        const btnMenu = document.getElementById('value-type');
        btnMenu.getElementsByClassName('text')[0].innerHTML = getTypeLable(this._type);
        this._menu = new Menu({
            el: btnMenu,
            items: [{
                text: getTypeLable(POWER),
                handler: (e) => this._onTypeChange(e, POWER)
            }, {
                text: getTypeLable(VOLTAGE),
                handler: (e) => this._onTypeChange(e, VOLTAGE)
            }],
            scope: this
        });
    }
}