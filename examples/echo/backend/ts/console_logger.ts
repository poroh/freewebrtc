//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Console log implementation of Logger
//

import { Logger } from './logger';

export class ConsoleLogger implements Logger {
    debug(str: string) {
        console.log('DEBUG', str);
    }
    info(str: string) {
        console.log('INFO ', str);
    }
    warn(str: string) {
        console.log('WARN ', str);
    }
    error(str: string) {
        console.log('ERROR', str);
    }
}
