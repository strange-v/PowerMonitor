export const POWER = 1;
export const VOLTAGE = 2;

export function getTypeLable(type) {
    switch (type) {
        case POWER:
            return 'Power';
        case VOLTAGE:
            return 'Voltage';
        default:
            throw 'Unknown type';
    }
}