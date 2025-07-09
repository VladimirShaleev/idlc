import sampleInit from 'sample';

console.log(`Sample ESM node.js`);

const sample = await sampleInit();

const result = sample.mul(3.2, 2.5)
console.log(`3.2 * 2.5 = ${result}`);

const vehicle = new sample.Vehicle("Truck");
vehicle.setVelocity({ x: 1.0, y: 2.0, z: 3.0 });
const dot = vehicle.dotVelocity({ x: 3.0, y: 2.0, z: 1.0 });

console.log(`Vehicle '${vehicle.name}' dot: ${dot}`);

vehicle.delete();
