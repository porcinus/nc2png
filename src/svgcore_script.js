function viewboxGetParams(svgElement){ //get values array of viewbox attribute
    let currentViewbox = svgElement.getAttribute('viewBox');
    let params = currentViewbox.split(' ');
    for (let i = 0; i < 4; i++){params[i] = parseFloat(params[i]);}
    return params;
}

function viewboxSetParams(svgElement, numArray){ //viewbox attribute from values array
    if (numArray[2] < 0.1){numArray[2] = 0.1;}
    if (numArray[3] < 0.1){numArray[3] = 0.1;}
    svgElement.setAttribute('viewBox', numArray.join(' '));
}

function cssSetRootProperty(svgElement, propertyName, value){ //update root css property
    document.documentElement.style.setProperty(propertyName, value);
}

function strokeScaling(svgBoundArray, viewBoxArray, strokeSize){ //compute new stroke size
    let scalingFactor = Math.min(svgBoundArray.width / viewBoxArray[2], svgBoundArray.height / viewBoxArray[3]);
    return strokeSize / scalingFactor * 5.;
}

//svg
var svg = document.querySelector('svg');

//backup initial viewport values
var viewBoxParamsInit = viewboxGetParams(svg);
var viewBoxMinSize = Math.min(viewBoxParamsInit[2], viewBoxParamsInit[3]);

//mouse wheel scaling factor
var wheelScaler = 10.;

//initial stroke width
var strokeDefaultWidthInit = .20;
var strokeGridWidthInit = .25;
var svgBound = svg.getBoundingClientRect();
cssSetRootProperty(svg, '--strokeWidthDefault', strokeScaling(svgBound, viewBoxParamsInit, strokeDefaultWidthInit));
cssSetRootProperty(svg, '--strokeWidthGrid', strokeScaling(svgBound, viewBoxParamsInit, strokeGridWidthInit));

var panView = false;
svg.addEventListener('mousedown', function(evt){if (evt.button == 0){panView = true;}}, false); //mouse left button press, start view pan
svg.addEventListener('mouseup', function(evt){if (evt.button == 0){panView = false;}}, false); //mouse left button release, stop view pan
svg.addEventListener('mouseleave', function(){panView = false;}, false); //mouse left svg element, stop view pan

svg.addEventListener('mousemove', function(evt){ //mouse move, pan view
    if (panView){
        let viewBoxParams = viewboxGetParams(svg);
        svgBound = svg.getBoundingClientRect();
        viewBoxParams[0] -= evt.movementX * (viewBoxParams[2] / svgBound.width);
        viewBoxParams[1] -= evt.movementY * (viewBoxParams[3] / svgBound.height);
        viewboxSetParams(svg, viewBoxParams); //update viewbox values
    }
}, false);

svg.addEventListener('wheel', function(evt){ //mouse wheel event, zoom
    let viewBoxParams = viewboxGetParams(svg);
    let zoomDirection = (evt.deltaY > 0) ? 1 : -1;
    let wheelScalingW = (viewBoxParams[2] / wheelScaler);
    let wheelScalingH = (viewBoxParams[3] / wheelScaler);

    viewBoxParams[2] += wheelScalingW * zoomDirection; //width
    viewBoxParams[3] += wheelScalingH * zoomDirection; //height
    viewboxSetParams(svg, viewBoxParams); //update viewbox values

    //update stroke widths according to new viewbox 'zoom'
    svgBound = svg.getBoundingClientRect();
    cssSetRootProperty(svg, '--strokeWidthDefault', strokeScaling(svgBound, viewBoxParams, strokeDefaultWidthInit));
    cssSetRootProperty(svg, '--strokeWidthGrid', strokeScaling(svgBound, viewBoxParams, strokeGridWidthInit));
}, false);

