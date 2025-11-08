#pragma once

/**
 * コマンドパターンの基底インターフェース
 * すべてのUI操作はこのインターフェースを実装する
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * コマンドを実行
     */
    virtual void execute() = 0;
};