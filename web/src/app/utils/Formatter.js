export default class Formatter {
    static uptime = (value) => {
        const days = Math.floor(value / (24 * 60 * 60));
        let hours = Math.floor(value / (60 * 60));
        const minutes = Math.floor(value / 60) % 60;
        const seconds = value % 60;

        let result = [{ value: hours, suffix: 'h' },
        { value: minutes, suffix: 'm' },
        { value: seconds, suffix: 's' }];

        if (days > 1) {
            hours = hours - days * 24;
            result = [{ value: days, suffix: 'd' },
            { value: hours, suffix: 'h' },
            { value: minutes, suffix: 'm' }];
        }

        return result
            .map(data => {
                return `${data.value}${data.suffix}`
            })
            .join(' ')
    }
    static power = (v) => this.autoNumber(v, 1, ['W', 'KW', 'MW'])
    static current = (v) => this.autoNumber(v, 2, ['A', 'kA'])
    static energy = (v) => this.autoNumber(v, 1, ['kWh', 'MWh'])
    static autoNumber = (value, scale, units) => {
        let unitText = '';
        const currentUnit = Math.floor((Number(value).toFixed(0).length - 1) / 3) * 3;
        const unitName = units[Math.floor(currentUnit / 3)];
        const num = value / ('1e' + currentUnit);

        unitText = `${this.number(num, scale)} ${unitName}`;

        return unitText;
    }
    static number = (v, scale, suffix = '') => {
        return v.toFixed(scale).replace(/([0-9]+(\.[0-9]+[1-9])?)(\.?0+$)/, '$1');
    }
}
