import {b2Fixture, b2PointState, b2Vec2} from "@box2d/core";

export class ContactPoint {
    public fixtureA!: b2Fixture;

    public fixtureB!: b2Fixture;

    public readonly normal = new b2Vec2();

    public readonly position = new b2Vec2();

    public state = b2PointState.b2_nullState;

    public normalImpulse = 0;

    public tangentImpulse = 0;

    public separation = 0;
}

let k_maxContactPoints = 2048
export { k_maxContactPoints }